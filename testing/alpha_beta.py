#!/usr/bin/env python3

import matplotlib.pyplot as plt

class Integrator:
    def __init__(self):
        self.integrand = 0
        self.dt = 1

    def input(self, x):
        self.integrand = self.integrand + (x*self.dt)

    def output(self):
        return self.integrand

class AlphaBetaFilter:
    def __init__(self):
        self.alpha = 0.7
        self.beta = 0.01

        self.alpha_integrator = Integrator()
        self.beta_integrator = Integrator()

    def update(self, expected_output):
        error = expected_output - self.alpha_integrator.output()

        beta_input = error * self.beta
        self.beta_integrator.input(beta_input)

        beta_output = self.beta_integrator.output()
        alpha_input = beta_output + error * self.alpha
        self.alpha_integrator.input(alpha_input)
        alpha_output = self.alpha_integrator.output()

        return {"position": alpha_output,
                "velocity": alpha_input,
                "acceleration": beta_input}


if __name__ == "__main__":
    filter = AlphaBetaFilter()
    values = [0, 0, 0, 1, 1, 1, 2, 2, 3, 3, 4, 4, 5, 6, 7, 7, 8, 9, 10, 10, 10, 9, 9, 9, 8, 8, 8, 8, 8, 8, 7, 7, 6, 6, 6, 6]
    results = [filter.update(val) for val in values]

    plot_x = range(len(values))

    positions = [result["position"] for result in results]
    plt.plot(plot_x, positions, 'b', plot_x, values, 'r')
    plt.show()
