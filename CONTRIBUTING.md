[JA2 Stracciatella](https://ja2-stracciatella.github.io/) is a community driven open source project that welcomes all. It has a few maintainers, who keep the big picture in mind, and a long list of contributors. The code can be forked, poked, twisted and hacked. It can be customized, extended, and collaboratively developed. You are invited to do so and join us!

That being said, not all contributions are welcome, since the project has a defined scope: a portable, cleaned-up JA2 with a limited set of (chocolate) additions. Large gameplay changes as seen in the 1.13 mod are therefore scoffed upon.

### Where to start?

Play the game and see what bothers you, what bug you hit, then report it and try to fix it. If you don't have an itch to scratch, we suggest looking at our bug tracker, for example the [help wanted](https://github.com/ja2-stracciatella/ja2-stracciatella/issues?q=is%3Aopen+is%3Aissue+label%3A%22help+wanted%22) list.

Instructions on building and IDE setup can be found in [COMPILATION](https://github.com/ja2-stracciatella/ja2-stracciatella/blob/master/COMPILATION.md).

If you are not a developer, there are many other possibilities which do not require programming skills to help JA2 Stracciatella to evolve. For example, you can help by retesting bug reports labelled with [retest](https://github.com/ja2-stracciatella/ja2-stracciatella/issues?q=is%3Aopen+is%3Aissue+label%3Aretest), help triage bugs, test solutions, research, create mods and more.

### Axioms of Style

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

### For maintainers

1. Merge a PR only after it has at least one other approval and it builds successfully on all buildbots (currently AppVeyor and GitHub Actions).
2. Squash merge only if the history is a mess or it makes more sense (eg. the whitespace sync PR).
3. For release planning check the milestones (copied over each time) and the [checklist](https://github.com/ja2-stracciatella/ja2-stracciatella/blob/master/docs/Release-checklist.md)
