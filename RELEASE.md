# Release Process

This document describes the step-by-step process for creating a new release of jig.

## Prerequisites

- Ensure all changes are committed and pushed to the `dev` branch
- Ensure all tests pass and the build is successful
- Have appropriate permissions to push to main branch and create releases

## Release Steps

### 1. Prepare the Release on Dev Branch

```bash
# Ensure you're on dev and up to date
git checkout dev
git pull origin dev

# Update the VERSION file with the new version number
echo "X.Y.Z" > VERSION

# Build the project to verify everything compiles
make clean
make

# Test the build
bin/jig --help

# Commit the version change
git add VERSION
git commit -m "chore: bump version to X.Y.Z"
git push origin dev
```

### 2. Merge to Main Branch

```bash
# Switch to main branch
git checkout main
git pull origin main

# Merge dev into main with --no-ff to preserve branch history
git merge --no-ff dev -m "release: merge dev for version X.Y.Z"
```

### 3. Tag the Release

```bash
# Create an annotated tag
git tag -a vX.Y.Z -m "Release version X.Y.Z"
```

### 4. Push Changes and Tags

```bash
# Push the main branch
git push origin main

# Push the tag
git push origin vX.Y.Z
```

### 5. Build Release Binaries

```bash
# Clean and rebuild for release
make clean
make

# Verify the binary
bin/jig --version  # or appropriate verification
```

### 6. Package the Release

```bash
# Create a release directory
mkdir -p release/jig-vX.Y.Z

# Copy binaries and essential files
cp -r bin release/jig-vX.Y.Z/
cp README.md release/jig-vX.Y.Z/
cp VERSION release/jig-vX.Y.Z/

# Create a tarball
cd release
tar -czf jig-vX.Y.Z-linux-x64.tar.gz jig-vX.Y.Z/
cd ..

# Calculate checksum
sha256sum release/jig-vX.Y.Z-linux-x64.tar.gz > release/jig-vX.Y.Z-linux-x64.tar.gz.sha256
```

### 7. Create GitHub Release

1. Go to the repository on GitHub
2. Click "Releases" → "Draft a new release"
3. Select the tag `vX.Y.Z`
4. Set release title: `jig vX.Y.Z`
5. Add release notes describing changes, fixes, and new features
6. Attach the following files:
   - `jig-vX.Y.Z-linux-x64.tar.gz`
   - `jig-vX.Y.Z-linux-x64.tar.gz.sha256`
7. Publish the release

### 8. Post-Release

```bash
# Switch back to dev branch for continued development
git checkout dev
git merge main
git push origin dev
```

## Version Numbering

Follow Semantic Versioning (SemVer):

- **MAJOR.MINOR.PATCH** (e.g., 1.2.3)
  - **MAJOR**: Incompatible API changes
  - **MINOR**: New functionality (backwards compatible)
  - **PATCH**: Bug fixes (backwards compatible)

## Release Checklist

- [ ] All changes merged and tested on `dev`
- [ ] VERSION file updated
- [ ] Build successful with no warnings
- [ ] Version committed to `dev` branch
- [ ] Merged to `main` with `--no-ff`
- [ ] Tagged with `vX.Y.Z`
- [ ] Pushed main branch and tags
- [ ] Binaries built and tested
- [ ] Release package created
- [ ] Checksums generated
- [ ] GitHub release created with notes
- [ ] Release assets uploaded
- [ ] Dev branch synced with main

## Hotfix Releases

For urgent fixes that bypass the normal dev workflow:

1. Create a hotfix branch from `main`: `git checkout -b hotfix/vX.Y.Z main`
2. Make the fix and update VERSION
3. Merge to `main` with `--no-ff`
4. Tag and release as normal
5. Merge the hotfix back to `dev`: `git checkout dev && git merge hotfix/vX.Y.Z`
