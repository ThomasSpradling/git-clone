# Simple Git Clone
Here is a sample implementation of `pit`, a place where you can throw snapshots of your current code-base and recover them later. This iteration of pit consists of tunnels (called *branches*).

This project is entirely present for education purposes to understand the inner-workings of more fully-featured `git`.

## Installation
Start by cloning this repo and moving into the directory:
```bash
git clone https://github.com/ThomasSpradling/git-clone.git
cd git-clone
```
From within this directory, run the following to leverage Makefile to build an executable for `pit`:
```bash
make pit
```
Now to run any command with `pit`, one can just use `./pit` from bash as in `./pit init`.

## Usage

Available commands:
- `pit init`: Creates a new repository.
- `pit add <file>`: Starts tracking the file `file`.
- `pit status`: Lists out all files that are currently being tracked.
- `pit rm <file>`: Untracks a file `file` currently being tracked.
- `pit commit -m <message>`: Commits all tracked files with message `message`.
- `pit log`: Prints all recent commits.
- `pit branch`: Prints all branches and puts a star on the current one.
- A few iterations of checkout:
  - `pit checkout <commit_id>`: Checks out a particular commit.
  - `pit checkout <branch>`: Checks out an existing branch's head.
  - `pit checkout -b <newbranch>`: Start a new branch at the current commit.

## Progress
- [x] ~~Initialization and tracking:~~ `init`, `add`, `status`, `rm`
- [x] ~~Committing:~~ `commit`, `log`
- [ ] Branching: `branch`, `checkout`