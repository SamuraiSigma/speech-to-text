import os  # system()


def can_build(platform):
    has_pulse = os.system("pkg-config --exists libpulse-simple") == 0
    has_alsa = os.system("pkg-config --exists alsa") == 0

    return platform == "x11" and (has_pulse or has_alsa)


def configure(env):
    pass
