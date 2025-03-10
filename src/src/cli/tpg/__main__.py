# __main__.py
import click

from . import commands
from . import config

@click.group()
@click.version_option()
@click.help_option()
@click.pass_context
def cli(ctx: click.Context):

    # define hyperparameters mapping and ref to the tpg base path
    ctx.obj = {
        "hyper_parameters": config.HYPER_PARAM_MAPPING,
        "tpg": config.TPG
    }

cli.add_command(commands.evolve)
cli.add_command(commands.plot)
cli.add_command(commands.replay)
cli.add_command(commands.clean)
cli.add_command(commands.kill)