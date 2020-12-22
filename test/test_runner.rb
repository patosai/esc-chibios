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
INCDIR = [SRC_DIR, STUBS_DIR, File.join(UNITY_DIR, "src")]

test_namespaces = ARGV.empty? ? ALL_TEST_NAMESPACES : ARGV

def namespace_to_test_file(file_namespace)
  return File.join(TEST_DIR, file_namespace + TEST_FILE_SUFFIX)
end

def namespace_to_source_file(file_namespace)
  return File.join(SRC_DIR, file_namespace + ".c")
end

def namespace_to_header_file(file_namespace)
  return File.join(SRC_DIR, file_namespace + ".h")
end

def get_tests_in_file(file_namespace)
  test_name_regex = /void (test_[A-Za-z_]+)\(void\)/
  tests = []
  File.open(namespace_to_test_file(file_namespace), "r") do |f|
    f.each_line do |line|
      if match = test_name_regex.match(line)
        tests << match[1]
      end
    end
  end
  tests
end

def generate_runner_file(file_namespace, random_str)
  test_names = get_tests_in_file(file_namespace)
  output = [
    "#include <unity.h>",
    "#include \"#{file_namespace}.h\""
  ] +
  test_names.map { |test_name| "void #{test_name}(void);" } + [
    "void setUp(void) {}",
    "void tearDown(void) {}",
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
  random_str = SecureRandom.hex(32)
  runner_filename = generate_runner_file(file_namespace, random_str)
  include_dirs = INCDIR.map { |dir| "-I#{dir}" }
  source_files = ALL_SOURCE_FILES
  source_files = ALL_TEST_NAMESPACES.map { |ns| namespace_to_test_file(ns) }
  source_files += ALL_STUB_SOURCE_FILES
  source_files << File.join(UNITY_DIR, "src", "unity.c")
  output_filename = File.join(BUILD_DIR, file_namespace + "_" + random_str + ".out")
  compile_command = "#{CC} #{include_dirs.join(' ')} #{runner_filename} #{source_files.join(' ')} -o #{output_filename}"
  run_test_command = "./#{output_filename}"
  puts "Compiling #{file_namespace} test..."
  succeeded_compiling = run_command(compile_command)
  if succeeded_compiling
    puts "Running #{file_namespace} test..."
    run_command(run_test_command)
  end
  return output_filename
end

def clean_build_dir
  if File.directory?(BUILD_DIR)
    FileUtils.rm(Dir.glob(File.join(BUILD_DIR, "*")))
  end
end

clean_build_dir
test_namespaces.each { |ns| compile_and_run_test(ns) }