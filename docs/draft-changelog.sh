#!/bin/bash
# fetches all issues in the selected milestone, parses their data and pretty-prints it
# depends on jq, gawk and curl
milestoneID=${1:-4} # grab it from frontend URL
baseURL="https://api.github.com/repos/ja2-stracciatella/ja2-stracciatella"

function fetch {
  curl -sH "Accept: application/vnd.github.v3+json" "$1"
}

# get issue count, since we'll need to use paging
count=$(fetch "$baseURL/milestones/$milestoneID" | jq '.open_issues + .closed_issues')

# split into chunks of 100, the API limit
page=1
n=100
url="$baseURL/issues?milestone=$milestoneID&state=all"
while (( count > 0 )); do
  fetch "$url&per_page=$n&page=$page" |
  jq '.[] | {title, number, labels: [.labels[].name] | tostring}'

  (( count -= n ))
  (( page++ ))
done | gawk '
/^{$/,/^}$/ {
  if (/^.$/) next
  # clean up the data
  $1 = ""
  title = gensub("^..(.*)..$", "\\1", "g")
  # also capitalize it and remove escaping
  title = substr(toupper(title), 1,1) substr(title, 2)
  title = gensub("\\\\", "", "g", title)
  getline
  number = gensub("^(.*),$", "#\\1", "g", $2) # wrong for PRs, but who cares
  getline

  labels = gensub("\\\\\"", "", "g", $2)
  if (labels ~ /invalid|duplicate/) next

  # analyze the labels to pretty-print everything at the end
  van = 0
  if (labels ~ /[^-]vanilla/) van = 1
  nonvan = index(labels, "non-vanilla")
  feature = 0
  if (/(feature|enhancement|proposal|task)/) feature = 1
  launcher = 0
  if (/launcher/ || title ~ /[lL]auncher/) launcher = 1
  editor = 0
  if (/editor/ || title ~ /[eE]ditor/) editor = 1
  bug = index(labels, "bug")
  maintenance = 0
  if (labels ~ /(maintenance|dependencies|build-system|release)/) maintenance = 1
  unset = 0
  if (length(labels) == 4) unset = 1

  format = "- %s: %s (%s)\n"
  if (van || nonvan) {
    format = "- %s (%s): %s (%s)\n"
    origin = "non-vanilla"
    if (van) origin = "vanilla"
  } else {
   origin = ""
  }
  type = "UNKNOWN"
  if (feature) {
    type = "Enhancement"
  } else if (launcher) {
    type = "Launcher"
  } else if (editor) {
    type = "Editor"
  } else if (bug) {
    type = "Bugfix"
  } else if (maintenance) {
    type = "Maintenance"
  } else if (unset) {
    type = "UNKNOWN"
  }

  if (origin) {
    printf format, type, origin, title, number
  } else {
    printf format, type, title, number
  }
}
' | sort -u

echo "-----------------------------------------------------------------------------"
echo "Remove the (non-vanilla) tag from bugfixes once untagged ones are categorized"
