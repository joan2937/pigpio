## Submitting a New Issue
If you are reporting a **bug** or defect, please provide the steps to reproduce the problem you are describing.
Ideally, provide a simple script that will reproduce the issue.  Also provide a description of the hardware and 
software types and versions used in your application/test environment.

Requests for a feature or **enhancement** to the library software will be treated at a lower priority due to the lack
of resources (contributors with skillset, knowledge of the library or time).  Unless your request has wide support
from the community it will be treated at a low priority.  If the repo's maintainer judges your request to be of value
then it will be labeled `enhancement`.  If additional resources are required, it will be tagged with `help wanted`.

If you simply have a **question**, consult the SUPPORT.md document.

## Creating a Pull Request
Contributions are welcome.  To save time it is best to have an open issue relating to what it is you want to contribute
and to discuss the prospects of getting your contribution accepted.

Your changes *must* pass the set of test files `x_*`.  Please indicate that you have run the test scripts successfuly or attach
a screen shot of the output from the test scripts.

In addition, you *should* provide updated or additional scripts that at least test the 'happy' paths of your code changes.  For 
larger changes the additional test cases will be considered mandatory.

Beginning 2020, this repo will follow a dual branch model: `master` is the stable branch that people use in production. A second branch, `develop`, is the first branch to receive merges from bug fixes and new features.  Only after we consider `develop` stable we merge it into the `master` branch and release the changes with a tagged version.

Adhering to the following process is the best way to get your work included in the project:

- Fork the project, clone your fork, and configure the remotes:
```
# Clone your fork of the repo into the current directory
git clone https://github.com/<your-username>/pigio.git

# Navigate to the newly cloned directory
cd pigpio

# Assign the original repo to a remote called "upstream"
git remote add upstream https://github.com/joan2937/pigpio.git
```
- If you cloned a while ago, get the latest changes from upstream:
```
git checkout develop
git pull upstream develop
```
- Create a new topic branch (off the develop branch) to contain your feature, change, or fix:
```
git checkout -b <topic-branch-name>
```
- Commit your changes.

- Locally merge (or rebase) the upstream dev branch into your topic branch:
```
git pull [--rebase] upstream develop
```
- Push your topic branch up to your fork:
```
git push origin <topic-branch-name>
```
- Open a Pull Request with a clear title and description.  See [creating a pull request from a fork](https://help.github.com/en/github/collaborating-with-issues-and-pull-requests/creating-a-pull-request-from-a-fork).
Make sure the base branch drop down menu is selecting 'develop'.
