# PoKeysLib on LinuxCNC

This repository is a fork of mbosnak's PoKeysLib, configured to build using the latest LinuxCNC ISO on a self-hosted runner. The repository is set up to automatically create GitHub issues with error descriptions and build logs in case of failures.

## Forking the Repository

1. Visit mbosnak's PoKeysLib repository on Bitbucket or on GitHub if available.
2. Create a fork of the repository under your GitHub account.

## Setting up a Self-Hosted Runner with LinuxCNC

To use the latest LinuxCNC environment for building, follow these steps to set up a self-hosted runner using the LinuxCNC ISO:

1. Prepare a machine (physical or virtual) and install Debian Buster from the LinuxCNC ISO.
2. Once installed, configure this machine as a self-hosted GitHub Actions runner by following the steps mentioned here:

```sh
mkdir actions-runner && cd actions-runner
curl -o actions-runner-linux-x64-2.300.0.tar.gz -L https://github.com/actions/runner/releases/download/v2.300.0/actions-runner-linux-x64-2.300.0.tar.gz
tar xzf ./actions-runner-linux-x64-2.300.0.tar.gz
./config.sh --url https://github.com/YOUR_GITHUB_USERNAME/YOUR_REPO --token YOUR_TOKEN
./svc.sh install
./svc.sh start
```

3. Ensure that the Debian Buster environment has all required build dependencies for LinuxCNC and PoKeysLib.

## Setting Up GitHub Actions Workflow

Once the self-hosted runner is set up, configure GitHub Actions to trigger builds, catch errors, and create GitHub issues with build logs if the build fails.

Create the `.github/workflows/build.yml` file with the following content:

```yaml
name: Build and Test PoKeysLib on LinuxCNC

on:
  push:
    branches:
      - main
  pull_request:
    branches:
      - main

jobs:
  build:
    runs-on: self-hosted  # Use the self-hosted LinuxCNC runner

    steps:
    - name: Checkout repository
      uses: actions/checkout@v2

    - name: Install dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y build-essential cmake git libncurses5-dev libreadline-dev \
        tcl8.6-dev tk8.6-dev bwidget libxmu-dev libglu1-mesa-dev libgl1-mesa-dev \
        libgtk2.0-dev python3-tk python3-dev libboost-python-dev libmodbus-dev \
        libusb-1.0-0-dev libudev-dev python3-serial python3-usb python3-numpy

    - name: Build PoKeysLib
      run: |
        mkdir build && cd build
        cmake ..
        make
      continue-on-error: true  # Continue to next steps even if the build fails

    - name: Save build logs
      run: |
        if [ -f build.log ]; then
          cat build.log
        fi
      if: failure()

    - name: Create GitHub Issue on Failure
      if: failure()
      run: |
        echo "Build failed, attaching logs."
        LOG_FILE="build.log"
        if [ -f $LOG_FILE ]; then
          gh issue create --title "Build Error: ${{ github.sha }}" --body-file $LOG_FILE --label "build-error" --assignee @your-username
        else
          echo "No build log found."
```

## Testing the Setup

Push a commit or open a pull request to trigger the GitHub Actions workflow. The workflow will run the build on your self-hosted runner and, if any errors occur, it will automatically create an issue on your repository with the build logs.

## Optional: Docker Integration (Using linuxcnc-docker)

If you want to incorporate Docker builds based on the repository linuxcnc-docker for testing purposes, you could set up additional Docker containers in the workflow:

Example for integrating linuxcnc-docker:

```yaml
- name: Build LinuxCNC Docker container
  run: |
    git clone https://github.com/jeffersonjhunt/linuxcnc-docker
    cd linuxcnc-docker
    docker-compose up -d
    docker exec -it linuxcnc-docker_container_name /bin/bash -c "mkdir build && cd build && cmake .. && make"
```

This integration could allow you to test the PoKeysLib build process inside a containerized LinuxCNC environment.

By following this approach, your GitHub repository will be a fork of mbosnak's PoKeysLib, and it will build using the latest LinuxCNC ISO on a self-hosted runner. Any build errors will automatically create an issue with the build logs attached for easier debugging.
