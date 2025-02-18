# __main__.py

import click

from . import commands

@click.group()
def cli():
    pass

cli.add_command(commands.evolve)
cli.add_command(commands.plot)
cli.add_command(commands.replay)