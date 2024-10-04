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
  schedule:
    - cron: '0 0 */14 * *'  # Run every two weeks

jobs:
  update-pokeyslib:
    runs-on: self-hosted  # Use the self-hosted LinuxCNC runner

    steps:
    - name: Checkout repository
      uses: actions/checkout@v2

    - name: Clone PoKeysLib repository
      run: |
        git clone https://bitbucket.org/mbosnak/pokeyslib pokeyslib

    - name: Commit and push updates
      run: |
        git config --global user.name "github-actions[bot]"
        git config --global user.email "github-actions[bot]@users.noreply.github.com"
        git add pokeyslib
        git commit -m "Update PoKeysLib subfolder"
        git push

  build:
    runs-on: self-hosted  # Use the self-hosted LinuxCNC runner
    needs: update-pokeyslib

    steps:
    - name: Checkout repository
      uses: actions/checkout@v2

    - name: Install dependencies
      run: |
        sudo apt-get update
        xargs -a prerequisites.txt sudo apt-get install -y

    - name: Authenticate GitHub CLI
      run: |
        echo "${{ secrets.GITHUB_TOKEN }}" | gh auth login --with-token

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

    - name: Build PoKeysLib for Linux
      run: |
        cd pokeyslib
        mkdir build && cd build
        cmake ..
        make
```

## Setting up GitHub CLI Authentication

To enable the `gh` command to create issues, you need to authenticate the GitHub CLI with a GitHub token. Follow these steps:

1. Install GitHub CLI by following the instructions [here](https://cli.github.com/manual/installation).
2. Authenticate the GitHub CLI with your GitHub account by running the following command and following the prompts:

```sh
gh auth login
```

3. Generate a GitHub token with the necessary permissions by following the instructions [here](https://docs.github.com/en/github/authenticating-to-github/creating-a-personal-access-token).
4. Configure the `gh` command to use the generated token by running the following command:

```sh
export GH_TOKEN=YOUR_GITHUB_TOKEN
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

## Using prerequisites.txt for Installing Dependencies

To simplify the installation of dependencies, you can use a `prerequisites.txt` file. Follow these steps:

1. Create a `prerequisites.txt` file in the root of your repository with the following content:

```
git
build-essential
libusb-1.0-0
libusb-1.0-0-dev
cmake
libncurses5-dev
libreadline-dev
tcl8.6-dev
tk8.6-dev
bwidget
libxmu-dev
libglu1-mesa-dev
libgl1-mesa-dev
libgtk2.0-dev
python3-tk
python3-dev
libboost-python-dev
libmodbus-dev
libudev-dev
python3-serial
python3-usb
python3-numpy
```

2. Update the GitHub Actions workflow to install dependencies using the `prerequisites.txt` file:

```yaml
- name: Install dependencies
  run: |
    sudo apt-get update
    xargs -a prerequisites.txt sudo apt-get install -y
```

## Marking Issues in GitHub

In GitHub, the concept of **marking issues** typically refers to using **labels**, **milestones**, **projects**, **assignees**, **linked issues**, **automatic marking with keywords**, and **notifications and mentions** to organize and track the progress or status of issues. Each of these features offers a different way to categorize and manage issues in a repository. Here’s a breakdown of how each feature works for marking issues:

### 1. **Labels**:
   - **Purpose**: Labels are used to categorize issues based on their type, priority, status, or any other criteria you want to track.
   - **How to Use**:
     - Navigate to the **Issues** tab in your repository.
     - Click on an issue to open it.
     - On the right-hand side, find the **Labels** section, and click to select or create a new label.
     - Labels are customizable and can include text like `bug`, `enhancement`, `help wanted`, `in progress`, etc.
   - **Features**:
     - Color-coded for easy identification.
     - Can filter issues by label.
     - Flexible for any categorization system.
   - **Best Practices**:
     - Establish a clear and consistent labeling system (e.g., label all bugs with `bug`, and use `enhancement` for new features).

### 2. **Milestones**:
   - **Purpose**: Milestones are used to group issues that are related to a specific goal or project phase (e.g., a release, sprint, or version).
   - **How to Use**:
     - In the **Issues** tab, click on **Milestones**.
     - Create a new milestone, giving it a name, description, and due date.
     - Once created, you can assign issues to this milestone.
   - **Features**:
     - Shows a progress bar that tracks the completion percentage based on the number of issues closed.
     - Can set deadlines and descriptions.
     - Useful for tracking progress toward releases or goals.

### 3. **Assignees**:
   - **Purpose**: Assignees help delegate responsibility for an issue to a specific person or a group of collaborators.
   - **How to Use**:
     - In an issue, you can find the **Assignees** section on the right-hand side.
     - Click to assign one or more collaborators to the issue.
   - **Features**:
     - Helps indicate who is responsible for resolving an issue.
     - You can filter issues by assignee to see what tasks are assigned to whom.
     - Allows multiple assignees (if more than one person is working on an issue).
   - **Best Practices**:
     - Always assign an owner (assignee) to ensure responsibility for resolving the issue.

### 4. **Projects**:
   - **Purpose**: GitHub Projects are used to organize issues in a Kanban-style board, making it easy to track the status and flow of tasks.
   - **How to Use**:
     - Go to the **Projects** tab of the repository.
     - Create a new project board (e.g., Kanban board with columns like `To Do`, `In Progress`, `Done`).
     - Drag and drop issues from one column to another as their status changes.
   - **Features**:
     - Provides a visual representation of the workflow.
     - Allows for project planning and tracking by moving issues between columns.
     - Can set custom columns for workflows (e.g., `Backlog`, `In Progress`, `QA`, `Done`).
   - **Best Practices**:
     - Use GitHub Projects for Agile workflows, such as sprints or stages, to visually track the progress of issues across stages.

### 5. **Linked Issues and Pull Requests**:
   - **Purpose**: You can mark relationships between issues and pull requests by linking them. This helps keep track of the work done to resolve an issue.
   - **How to Use**:
     - In an issue or pull request, scroll down to the **Linked issues** section.
     - Link the related issues or PRs, which shows the relationship and progress.
   - **Features**:
     - Automatically closes issues when linked pull requests are merged (using keywords like `Fixes #issue_number` in PR descriptions).
     - Helps track which code changes resolve specific issues.
   - **Best Practices**:
     - Whenever a pull request is created to address an issue, make sure it is linked properly to automatically close the issue upon merging.

### 6. **Automatic Marking with Keywords**:
   - **Purpose**: You can use special keywords in pull request descriptions or commit messages to automatically close or mark issues.
   - **How to Use**:
     - In a PR or commit message, use keywords like `Fixes`, `Resolves`, or `Closes` followed by the issue number (e.g., `Fixes #123`).
   - **Features**:
     - When the pull request is merged, the linked issue is automatically closed.
     - Keeps issues and code changes tightly integrated.

### 7. **Notifications and Mentions**:
   - **Purpose**: Mark an issue for attention by mentioning someone or requesting a review.
   - **How to Use**:
     - In the issue comment section, use the `@` symbol followed by a username (e.g., `@username`).
     - The mentioned person will receive a notification, making it easier to get their attention.
   - **Features**:
     - Helps involve specific people in the discussion of an issue.
     - Can also request a review if necessary.

### Best Practices:
- **Consistent Labeling**: Establish a clear and consistent labeling system (e.g., label all bugs with `bug`, and use `enhancement` for new features).
- **Clear Milestones**: Define milestones based on significant project goals, like upcoming releases or major features, and assign issues accordingly.
- **Assign Ownership**: Always assign an owner (assignee) to ensure responsibility for resolving the issue.
- **Link Pull Requests**: Whenever a pull request is created to address an issue, make sure it is linked properly to automatically close the issue upon merging.
- **Use Projects for Agile Workflows**: If working in sprints or stages, use GitHub Projects to visually track the progress of issues across stages.

By leveraging these marking features effectively, you can streamline issue management and ensure better communication within your team.

## Subfolder Structure

The source from the repository `https://bitbucket.org/mbosnak/pokeyslib` is placed in a subfolder named `pokeyslib`. This subfolder is automatically updated every two weeks to ensure that the latest changes from the original repository are included.

## Automatic Update Mechanism

The `.github/workflows/build.yml` file includes a scheduled job to update the content from the specified repository every two weeks. This job clones the repository `https://bitbucket.org/mbosnak/pokeyslib` into the `pokeyslib` subfolder and commits the updates to the repository.

The update job is triggered by the `schedule` event, which is set to run at a two-week interval. This ensures that the `pokeyslib` subfolder is always up-to-date with the latest changes from the original repository.

## Compiling PoKeysLib for Linux

To compile PoKeysLib for Linux, follow these steps:

1. Navigate to the `pokeyslib` subfolder:

```sh
cd pokeyslib
```

2. Create a build directory and navigate into it:

```sh
mkdir build && cd build
```

3. Run CMake to configure the build:

```sh
cmake ..
```

4. Build the project:

```sh
make
```

These steps will compile PoKeysLib for Linux.
