import subprocess

def get_project_root_directory():
    # assumes that git is installed
    cmd="git rev-parse --show-toplevel"
    process = subprocess.Popen(
        cmd.split(),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE
    )
    stdout, _ = process.communicate()
    return stdout.decode("utf-8").strip()