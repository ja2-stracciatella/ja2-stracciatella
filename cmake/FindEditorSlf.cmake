# Finds the latest release of the free editor.slf.
# see https://github.com/ja2-stracciatella/free-ja2-resources

# The module defines the following variables:
#   EDITORSLF_FOUND - TRUE if a release was found
#   EDITORSLF_NAME - name of the release
#   EDITORSLF_VERSION - version of the release
#   EDITORSLF_URL - url to the editor.slf file
#   EDITORSLF_MD5_URL - url to the MD5 hash of the editor.slf file
# Example usage:
#   find_package(EditorSlf REQUIRED)


include(FindPackageHandleStandardArgs)
include(JSONParser)
include(CurlFunctions)

#################
# latest release

set(headers)
if(NOT "$ENV{GITHUB_ACCESS_TOKEN}" STREQUAL "")
    list(APPEND headers "Authorization: token $ENV{GITHUB_ACCESS_TOKEN}")
endif()
curl_download(
    URL "https://api.github.com/repos/ja2-stracciatella/free-ja2-resources/releases/latest"
    FILE "${CMAKE_CURRENT_BINARY_DIR}/latest.json"
    VAR latest_json
    HEADERS ${headers}
)

sbeParseJson(release latest_json)
set(EDITORSLF_NAME "${release.name}")
string(SUBSTRING "${release.tag_name}" 1 -1 EDITORSLF_VERSION)
foreach(idx ${release.assets})
    set(name "${release.assets_${idx}.name}")
    set(url "${release.assets_${idx}.browser_download_url}")
    if(name STREQUAL "editor.slf")
        message("-- slf url: ${url}")
        set(EDITORSLF_URL "${url}")
    elseif (name STREQUAL "editor.slf.md5")
        message("-- md5 url: ${url}")
        set(EDITORSLF_MD5_URL "${url}")
    endif()
endforeach(idx)
sbeClearJson(release)


#################
# package output

find_package_handle_standard_args(EditorSlf
    REQUIRED_VARS EDITORSLF_NAME EDITORSLF_VERSION EDITORSLF_URL EDITORSLF_MD5_URL
    VERSION_VAR EDITORSLF_VERSION
)
