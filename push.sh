#!/usr/bin/env sh

commit_message=$(date +"%d-%b-%y@%H:%M:%S")

git add .
git commit -m "$commit_message"
git push --set-upstream origin main
