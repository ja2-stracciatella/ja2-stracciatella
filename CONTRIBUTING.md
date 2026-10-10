# Contributing

Play the game and see what bothers you. If you find a bug, [report it](https://github.com/ja2-stracciatella/ja2-stracciatella/issues/new/choose).

If you are not a developer, there are many other possibilities which do not require programming skills to help JA2 Stracciatella to evolve. For example, you can help by retesting bug reports labelled with [retest](https://github.com/ja2-stracciatella/ja2-stracciatella/issues?q=is%3Aopen+is%3Aissue+label%3Aretest), help triage bugs, test solutions, research, write documentation, create mods and more.

## Where to start?

If you don't have an itch to scratch, we suggest looking at our [bug tracker](https://github.com/ja2-stracciatella/ja2-stracciatella/issues), for example the [help wanted](https://github.com/ja2-stracciatella/ja2-stracciatella/issues?q=is%3Aopen+is%3Aissue+label%3A%22help+wanted%22) list.

Instructions on building and IDE setup can be found in [COMPILATION](COMPILATION.md). Read our [docs](docs) directory, where code-specific documentation can be generated locally.

## Contributing code

Pull requests with bug fixes are very welcome. That being said, **not all code contributions are welcome**, since the project has a defined scope: a portable, cleaned-up JA2 with a limited set of (chocolate) additions. Large gameplay changes as seen in the 1.13 mod are therefore scoffed upon.

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
