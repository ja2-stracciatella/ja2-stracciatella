# Contributing

There are many ways to help:

* Play-test [nightly](https://ja2-stracciatella.github.io/download/#nightlies) development versions of JA2 Stracciatella.
* [Report bugs](https://github.com/ja2-stracciatella/ja2-stracciatella/issues/new/choose) or suggest enhancements.
* Help triage bug reports.
* [Re-test](https://github.com/ja2-stracciatella/ja2-stracciatella/issues?q=is%3Aopen+is%3Aissue+label%3Aretest) bug reports that could use additional information.
* Test proposed [pull request](https://github.com/ja2-stracciatella/ja2-stracciatella/pulls) solutions.
* Write documentation
* Create mods

And many more!

## Contributing code

Fixes and modernizations are very welcome. Large gameplay overhauls (such as in the 1.13 mod) will quite likely not be accepted. Instead, we invite changes to the modding interface (Lua bindings, data externalizations) which allow for reimagined gameplay through 3rd-party mods.

Browse our [bug tracker](https://github.com/ja2-stracciatella/ja2-stracciatella/issues) for ideas on what can be fixed. For example, the [help wanted](https://github.com/ja2-stracciatella/ja2-stracciatella/issues?q=is%3Aopen+is%3Aissue+label%3A%22help+wanted%22) list.

Read our [technical docs](docs) directory, where C++ documentation can be generated. Build and IDE setup instructions can be found in [COMPILATION](COMPILATION.md).

### How to make a pull request

1. [Fork](https://docs.github.com/pull-requests/how-tos/work-with-forks/fork-a-repo) the project.
2. [Create](https://github.com/git-guides#create-a-branch) a branch (`git checkout -b my_feature_branch`).
3. Write and test your new code logic on the branch.
4. [Add](https://github.com/git-guides/git-add) your changes to stage them.
5. [Commit](https://github.com/git-guides/git-commit) your changes (`git commit -am "description of your changes"`).
6. [Push](https://github.com/git-guides/git-push) to the branch (`git push origin my_feature_branch`).
7. [Create a pull request](https://docs.github.com/pull-requests/how-tos/create-pull-requests/creating-a-pull-request) from your branch into `master`.

### Axioms of Style

Please don't reformat the code for the sake of it, because it will make the merge process harder. Instead, use the following settings in your editor:

- Display tabs as 8 spaces.
- Indent with tabs.

If you add new code, please don't add spaces after opening or before closing parentheses.

1. When in doubt, follow the style of the existing function or file.
1.1. When creating a new file, follow the style of existing related files
1.1.1. Do not forget to include the license header.
2. Code indentation is done with single tabulators.
2.1. Don't add spaces after opening or before closing parentheses.
3. Try to avoid creating very long lines. There is no set maximum.
4. Sort includes by type (project, system) and alphabetically.
5. Avoid changing or fixing the style just for the sake of it.
6. Dead code should be removed, not disabled with comments or a macro directive.

### Version tracking

1. Split your changes (commits) into well-rounded units of logic (git commit -p can help).
2. Each commit should compile and run.
3. Commit messages should be descriptive (why is more important than what).
4. Rebasing and force pushing to pull request branches is fine.

All of this makes reviewing and bisecting for regressions easier.

### Miscellaneus

1. New code should have tests if possible.

### For developers with commit access

0. The master branch is considered protected, meaning changes are expected to go in through PRs.
1. Merge a PR only after it has at least one other approval and it builds successfully on all buildbots (or the failures are known to be unrelated).
2. Squash merge only if the history is a mess or it makes more sense (eg. the whitespace sync PR).
3. For release planning check the milestones and the [checklist](https://github.com/ja2-stracciatella/ja2-stracciatella/blob/master/docs/Release-checklist.md)
