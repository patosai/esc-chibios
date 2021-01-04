#!/usr/bin/env ruby

require 'fileutils'
require 'open3'
require 'securerandom'

TEST_DIR = File.dirname(__FILE__)
SRC_DIR = File.expand_path(File.join(TEST_DIR, "..", "src"))
UNITY_DIR = File.join(TEST_DIR, "Unity")
BUILD_DIR = File.join(TEST_DIR, "build")
STUBS_DIR = File.join(TEST_DIR, "stubs")

TEST_FILE_SUFFIX = ".test.c"
TEST_NAMESPACE_REGEX = Regexp.new("([a-z_]+)" + TEST_FILE_SUFFIX)

ALL_SOURCE_FILES = Dir.glob(File.join(SRC_DIR, "*.c"))
ALL_TEST_NAMESPACES = Dir.glob(File.join(TEST_DIR, "*" + TEST_FILE_SUFFIX)).map { |filename| TEST_NAMESPACE_REGEX.match(filename)[1]}
ALL_STUB_SOURCE_FILES = Dir.glob(File.join(STUBS_DIR, "*.c"))

CC = "gcc"
CCFLAGS = ""
LDFLAGS = "-lm"
INCDIR = [SRC_DIR, STUBS_DIR, File.join(UNITY_DIR, "src")]

def namespace_to_test_file(file_namespace)
  return File.join(TEST_DIR, file_namespace + TEST_FILE_SUFFIX)
end

def namespace_to_source_file(file_namespace)
  return File.join(SRC_DIR, file_namespace + ".c")
end

def namespace_to_header_file(file_namespace)
  return File.join(SRC_DIR, file_namespace + ".h")
end

def test_namespace_data(file_namespace)
  test_name_regex = /void (test_[A-Za-z_]+)\(void\)/
  setup_regex = /void setUp\(void\)/
  teardown_regex = /void tearDown\(void\)/
  tests = []
  has_setup = false
  has_teardown = false
  File.open(namespace_to_test_file(file_namespace), "r") do |f|
    f.each_line do |line|
      if match = test_name_regex.match(line)
        tests << match[1]
      end
      if !has_setup && setup_regex.match(line)
        has_setup = true
      end
      if !has_teardown && teardown_regex.match(line)
        has_teardown = true
      end
    end
  end
  {tests: tests,
   has_setup: has_setup,
   has_teardown: has_teardown}
end

def generate_runner_file(file_namespace, random_str)
  data = test_namespace_data(file_namespace)
  test_names = data[:tests]
  has_setup = data[:has_setup]
  has_teardown = data[:has_teardown]
  output = [
    "#include <unity.h>",
    "#include \"#{file_namespace}.h\"",
  ] +
  test_names.map { |test_name| "void #{test_name}(void);" } + [
    has_setup ? "void setUp(void);" : "void setUp(void) {}",
    has_teardown ? "void tearDown(void);" : "void tearDown(void) {}",
    "int main(void) {",
    "  UNITY_BEGIN();"
  ] +
  test_names.map { |test_name| "  RUN_TEST(#{test_name});" } + [
    "  return UNITY_END();",
    "}"
  ]
  output = output.join("\n")
  new_filename = "#{file_namespace}_runner_" + random_str + ".c"
  complete_filename = File.join(BUILD_DIR, new_filename)
  Dir.mkdir(BUILD_DIR) unless Dir.exist?(BUILD_DIR)
  File.write(complete_filename, output)
  return complete_filename
end

def run_command(cmd)
  IO.popen(cmd) do |io|
    while (line = io.gets) do
      puts line
    end
  end
  $?.success?
end

def compile_and_run_test(file_namespace)
  random_str = SecureRandom.hex(8)
  runner_filename = generate_runner_file(file_namespace, random_str)
  include_dirs = INCDIR.map { |dir| "-I#{dir}" }
  source_files = [
    # could definitely be smarter here about which source files to compile
    namespace_to_source_file(file_namespace),
    namespace_to_test_file(file_namespace),
    File.join(UNITY_DIR, "src", "unity.c")
  ]
  source_files += ALL_STUB_SOURCE_FILES
  output_filename = File.join(BUILD_DIR, file_namespace + "_" + random_str + ".out")

  puts ""
  puts "Compiling #{file_namespace} test"
  compile_command = "#{CC} #{CCFLAGS} #{include_dirs.join(' ')} #{runner_filename} #{source_files.join(' ')} -o #{output_filename} #{LDFLAGS}"
  succeeded_compiling = run_command(compile_command)

  if succeeded_compiling
    puts "Running #{file_namespace} test"
    run_command("#{File.expand_path(output_filename)}")
  end
  return output_filename
end

task default: :test

task :clean do |t|
  puts "Cleaning build directory"
  if File.directory?(BUILD_DIR)
    FileUtils.rm(Dir.glob(File.join(BUILD_DIR, "*")))
  end
end

task test: [:clean] do |t, args|
  args = args.to_a
  test_namespaces = args.empty? ? ALL_TEST_NAMESPACES : args
  test_namespaces.each { |ns| compile_and_run_test(ns) }
end
