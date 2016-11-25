#!/bin/sh

git status
echo "git add all change"
git add -A
echo "git commit"
git commit
echo "git push"
git push origin master
echo "git see history"
git logall -n10
