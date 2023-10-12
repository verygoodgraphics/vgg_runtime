# VGG Contribution Guidelines

We sincerely appreciate your interest in and use of VGG. We welcome for issues, pull requests, or engaging in discussions on our [Discord channel](https://discord.gg/g3HJuKP54D). Before that, please read [Code of Conduct](CODE_OF_CONDUCT.md).

## Create an Issue

If you find any bug including a typo or a runtime bug, feel free to create an issue.

If it's a minor bug that is easy to fix, you can create a [pull request](#pull-request) directly, bypassing this step.

1. Check [issue list](https://github.com/verygoodgraphics/vgg_runtime/issues) to avoid duplicated issues.
2. If it's security related, please report to [VGG](mailto:bd@verygoodgraphics.com) directly.
3. Follow the issue template, and fill in information as much as you can.

We will reply ASAP.

## Solve an Issue

If you are capable to solve an unassigned issue in [#help-wanted](https://github.com/verygoodgraphics/vgg_runtime/labels/help%20wanted), you can reply under the comment thread, waiting for an assignment. You can create a [pull request](#pull-request) once the issue is resovled.

## Pull Request

We welcome and appreciate your contributions in terms of bug fixes, enhancements to documents, or creation of new features. Feel free to send a pull request (PR).

Both core team members and external contributors send pull requests that undergo the same review process. Refer to [GitHub's pull request tutorial](https://docs.github.com/en/pull-requests/collaborating-with-pull-requests) if you encounter any issues.

Upon opening a pull request, you will be prompted to sign a [Contributor License Agreement (CLA)](.github/CLA.md) by leaving a comment as instructed.

### Basic PR Requirements

- Keep each commit atomic, and commit with clear messages.
- Follow the PR template, and write clear descriptions.
- For C++ code, we enforce the coding style via clang-format and clang-tidy.
- Make sure the file extensions are *.hpp or *.cpp. Strictly use CamelCase in filenames.
- Make sure you put useful comments in your code.
- Make sure your code compiles and passes all uint tests. Please refer to [Build Instructions](README.md#build-instructions) for details.

## Need More Help?

Please contact us through
- Discord: https://discord.gg/g3HJuKP54D
- Twitter: https://twitter.com/VGG_Design
- E-mail: bd@verygoodgraphics.com
