# commands.py

import click

@click.command(help="Evolve a policy")
def evolve():
    click.echo("Evolve")

@click.command(help="Plot results for an experiment")
def plot():
    click.echo("Plot")

@click.command(help="Replay the best performing policy")
def replay():
    click.echo("Replay")