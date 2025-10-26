# arduino-common

Common libraries for Arduino projects

## Initial setup in a project

```bash
git submodule add <repo-url> lib/common
```

## Clone project elsewhere

If you clone your project, run the `submodule update`:

```bash
git clone <project-url>
git submodule update --init --recursive
```

## Update shared library

To get the last version:

```bash
cd lib/common
git pull origin main
cd ../..
git add lib/common
git commit -m "Update common library"
```

Or if you have a Makefile:

```
.PHONY: update-libs
update-libs:
	git submodule update --remote --merge
```
