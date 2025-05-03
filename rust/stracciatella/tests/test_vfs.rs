//! This file contains tests for the `stracciatella::vfs` module.

// Vfs tests.
mod vfs {
    #[test]
    fn read() {
        let (temp, dir, dir_fs) = create_temp_dir();
        create_data_slf(&dir); // data.slf

        let mut vfs = Vfs::new();
        add_slf(&mut vfs, &dir_fs, "data.slf");
        let data = read_file_data(&vfs, "foo.txt");
        assert_eq!(&data, b"data.slf");

        temp.close().expect("close temp dir");
    }

    #[test]
    fn read_in_layer() {
        let (temp, dir, dir_fs) = create_temp_dir();
        create_file(&dir.join("foo/bar/dironly.txt"));
        create_file(&dir.join("foo/bar/baz.txt"));
        create_foo_slf(&dir);
        create_foobar_slf(&dir);

        let mut vfs = Vfs::new();
        vfs.add_dir(&dir).expect("dir");
        add_slf(&mut vfs, &dir_fs, "foo.slf");
        add_slf(&mut vfs, &dir_fs, "foobar.slf");

        // File exists in dir layer
        assert_eq!(
            read_file_data_in_layer(&vfs, 0, "foo/bar/dironly.txt")
                .expect("read in layer")
                .expect("content"),
            b"dironly.txt"
        );
        // File exists in multiple layers
        assert_eq!(
            read_file_data_in_layer(&vfs, 0, "foo/bar/baz.txt")
                .expect("read in layer")
                .expect("content"),
            b"baz.txt"
        );
        assert_eq!(
            read_file_data_in_layer(&vfs, 1, "foo/bar/baz.txt")
                .expect("read in layer")
                .expect("content"),
            b"foo.slf"
        );
        assert_eq!(
            read_file_data_in_layer(&vfs, 2, "foo/bar/baz.txt")
                .expect("read in layer")
                .expect("content"),
            b"foobar.slf"
        );
        // File does not exist
        assert!(read_file_data_in_layer(&vfs, 0, "foo/foo/baz.txt")
            .expect("read in layer")
            .is_none());
        // File is a directory
        assert!(read_file_data_in_layer(&vfs, 0, "foo")
            .expect("read in layer")
            .is_none());

        temp.close().expect("close temp dir");
    }

    #[test]
    fn read_patched_json() {
        let (temp, dir, _) = create_temp_dir();

        // Invalid json
        create_file(&dir.join("layer1/invalid.json"));

        // Without patches should still work
        create_json_file(
            &dir.join("layer1/without-patches.json"),
            &json!({ "layer1": true }),
        );
        create_json_file(
            &dir.join("layer3/without-patches.json"),
            &json!({ "layer3": true }),
        );

        // All of the patches should apply
        create_json_file(&dir.join("layer1/basic.json"), &json!({}));
        create_json_file(
            &dir.join("layer1/basic.patch.json"),
            &json!([ { "op": "add", "path": "/layer1", "value": true } ]),
        );
        create_json_file(
            &dir.join("layer2/basic.patch.json"),
            &json!([ { "op": "add", "path": "/layer2", "value": true } ]),
        );
        create_json_file(
            &dir.join("layer3/basic.patch.json"),
            &json!([ { "op": "add", "path": "/layer3", "value": true } ]),
        );

        // Patch order should be correct
        create_json_file(&dir.join("layer1/order.json"), &json!([]));
        create_json_file(
            &dir.join("layer1/order.patch.json"),
            &json!([ { "op": "add", "path": "/-", "value": "layer1" } ]),
        );
        create_json_file(
            &dir.join("layer2/order.patch.json"),
            &json!([ { "op": "add", "path": "/-", "value": "layer2" } ]),
        );
        create_json_file(
            &dir.join("layer3/order.patch.json"),
            &json!([ { "op": "add", "path": "/-", "value": "layer3" } ]),
        );

        // Only the higher level patches than the last full file should apply
        create_json_file(&dir.join("layer1/higher-level.json"), &json!({}));
        create_json_file(
            &dir.join("layer1/higher-level.patch.json"),
            &json!([ { "op": "add", "path": "/layer1", "value": true } ]),
        );
        create_json_file(&dir.join("layer2/higher-level.json"), &json!({}));
        create_json_file(
            &dir.join("layer2/higher-level.patch.json"),
            &json!([ { "op": "add", "path": "/layer2", "value": true } ]),
        );
        create_json_file(
            &dir.join("layer3/higher-level.patch.json"),
            &json!([ { "op": "add", "path": "/layer3", "value": true } ]),
        );

        let mut vfs = Vfs::new();
        vfs.add_dir(&dir.join("layer3")).expect("layer3");
        vfs.add_dir(&dir.join("layer2")).expect("layer2");
        vfs.add_dir(&dir.join("layer1")).expect("layer1");

        // The sucess cases that were described above
        assert_eq!(
            vfs.read_patched_json(&Nfc::caseless_path("without-patches.json"))
                .expect("read patched json"),
            json!({ "layer3": true })
        );
        assert_eq!(
            vfs.read_patched_json(&Nfc::caseless_path("basic.json"))
                .expect("read patched json"),
            json!({ "layer1": true, "layer2": true, "layer3": true })
        );
        assert_eq!(
            vfs.read_patched_json(&Nfc::caseless_path("order.json"))
                .expect("read patched json"),
            json!(["layer1", "layer2", "layer3"])
        );
        assert_eq!(
            vfs.read_patched_json(&Nfc::caseless_path("higher-level.json"))
                .expect("read patched json"),
            json!({ "layer2": true, "layer3": true })
        );

        // Error cases
        assert_eq!(
            vfs.read_patched_json(&Nfc::caseless_path("nonexistant.sti"))
                .expect_err("error")
                .to_string(),
            "patched json must end in .json extension"
        );
        assert_eq!(
            vfs.read_patched_json(&Nfc::caseless_path("nonexistant.json"))
                .expect_err("error")
                .to_string(),
            "entity not found"
        );
        assert!(vfs
            .read_patched_json(&Nfc::caseless_path("invalid.json"))
            .expect_err("error")
            .to_string()
            .contains("failed to deserialize json"));

        temp.close().expect("close temp dir");
    }

    #[test]
    fn exists() {
        let (temp, dir, dir_fs) = create_temp_dir();
        create_file(&dir.join("foo/foo1.txt"));
        create_file(&dir.join("foo/Foo2.txt"));
        create_foo_slf(&dir);

        let mut vfs = Vfs::new();
        vfs.add_dir(&dir).expect("dir");
        add_slf(&mut vfs, &dir_fs, "foo.slf");

        assert!(vfs
            .exists(&Nfc::caseless_path("foo/foo1.txt"))
            .expect("exists should work"));
        assert!(vfs
            .exists(&Nfc::caseless_path("foo/foo2.txt"))
            .expect("exists should work"));
        assert!(vfs
            .exists(&Nfc::caseless_path("foo"))
            .expect("exists should work"));
        assert!(vfs
            .exists(&Nfc::caseless_path("Foo"))
            .expect("exists should work"));
        assert!(vfs
            .exists(&Nfc::caseless_path("foo/"))
            .expect("exists should work"));
        assert!(vfs
            .exists(&Nfc::caseless_path("Foo/"))
            .expect("exists should work"));
        assert!(!vfs
            .exists(&Nfc::caseless_path("foo2.txt"))
            .expect("exists should work"));

        temp.close().expect("close temp dir");
    }

    #[test]
    fn read_layers_for_file() {
        let (temp, dir, dir_fs) = create_temp_dir();
        create_file(&dir.join("foo/bar/dironly.txt"));
        create_file(&dir.join("root.txt"));
        create_foo_slf(&dir);
        create_foobar_slf(&dir);

        let mut vfs = Vfs::new();
        vfs.add_dir(&dir).expect("dir");
        add_slf(&mut vfs, &dir_fs, "foo.slf");
        add_slf(&mut vfs, &dir_fs, "foobar.slf");

        // Root path
        assert_eq!(
            vfs.read_layers(&Nfc::caseless_path("root.txt"))
                .expect("read layers"),
            vec![0]
        );
        assert_eq!(
            vfs.read_layers(&Nfc::caseless_path("Root.txt"))
                .expect("read layers"),
            vec![0]
        );

        // Specific file
        assert_eq!(
            vfs.read_layers(&Nfc::caseless_path("foo/bar.txt"))
                .expect("read layers"),
            vec![1]
        );
        assert_eq!(
            vfs.read_layers(&Nfc::caseless_path("foo/Bar.txt"))
                .expect("read layers"),
            vec![1]
        );
        assert_eq!(
            vfs.read_layers(&Nfc::caseless_path("foo/bar/baz.txt"))
                .expect("read layers"),
            vec![1, 2]
        );
        assert_eq!(
            vfs.read_layers(&Nfc::caseless_path("foo/bar/baz.txt"))
                .expect("read layers"),
            vec![1, 2]
        );
        assert_eq!(
            vfs.read_layers(&Nfc::caseless_path("foo/bar/dironly.txt"))
                .expect("read layers"),
            vec![0]
        );
        assert_eq!(
            vfs.read_layers(&Nfc::caseless_path("foo/bar/dironly.txt"))
                .expect("read layers"),
            vec![0]
        );

        // Directory
        assert_eq!(
            vfs.read_layers(&Nfc::caseless_path("foo"))
                .expect("read layers"),
            vec![0, 1, 2]
        );
        assert_eq!(
            vfs.read_layers(&Nfc::caseless_path("Foo"))
                .expect("read layers"),
            vec![0, 1, 2]
        );

        // Nonexistant stuff
        assert!(vfs
            .read_layers(&Nfc::caseless_path("nonexistant.txt"))
            .expect("read layers")
            .is_empty());
        assert!(vfs
            .read_layers(&Nfc::caseless_path("foo/nonexistant.txt"))
            .expect("read layers")
            .is_empty());

        temp.close().expect("close temp dir");
    }

    #[test]
    fn seek() {
        let (temp, dir, dir_fs) = create_temp_dir();
        create_data_slf(&dir); // data.slf

        let mut vfs = Vfs::new();
        add_slf(&mut vfs, &dir_fs, "data.slf");
        let mut file = vfs.open(&Nfc::caseless_path("foo.txt")).unwrap();

        assert_eq!(file.seek(SeekFrom::End(0)).unwrap(), 8);
        assert_eq!(file.seek(SeekFrom::Current(4)).unwrap(), 12);
        assert_eq!(file.seek(SeekFrom::Current(-8)).unwrap(), 4);
        assert!(file.seek(SeekFrom::Current(-5)).is_err());
        assert_eq!(file.seek(SeekFrom::Start(0)).unwrap(), 0);

        temp.close().expect("close temp dir");
    }

    #[test]
    fn order() {
        let (temp, dir, dir_fs) = create_temp_dir();
        create_foo_slf(&dir); // foo.slf
        create_foobar_slf(&dir); // foobar.slf
        create_file(&dir.join("foo/bar/baz.txt")); // baz.txt

        let mut vfs = Vfs::new();
        vfs.add_dir(&dir).expect("dir");
        add_slf(&mut vfs, &dir_fs, "foo.slf");
        add_slf(&mut vfs, &dir_fs, "foobar.slf");

        let data = read_file_data(&vfs, "foo/bar/baz.txt");
        assert_eq!(&data, b"baz.txt");

        temp.close().expect("close temp dir");
    }

    #[test]
    fn paths() {
        let (temp, dir, dir_fs) = create_temp_dir();
        create_foo_slf(&dir); // foo.slf

        let mut vfs = Vfs::new();
        add_slf(&mut vfs, &dir_fs, "foo.slf");
        // case insensitive
        let data = read_file_data(&vfs, "FOO/bar.txt");
        assert_eq!(&data, b"foo.slf");
        let data = read_file_data(&vfs, "foo/BAR.TXT");
        assert_eq!(&data, b"foo.slf");
        let data = read_file_data(&vfs, "foo/BAR/baz.txt");
        assert_eq!(&data, b"foo.slf");
        let data = read_file_data(&vfs, "foo/bar/BAZ.TXT");
        assert_eq!(&data, b"foo.slf");
        let data = read_file_data(&vfs, "foo\\bar/ὈΔΥΣΣΕΎΣ.baz");
        assert_eq!(&data, b"foo.slf");
        // separators
        let data = read_file_data(&vfs, "foo/bar/baz.txt");
        assert_eq!(&data, b"foo.slf");
        let data = read_file_data(&vfs, "foo/bar\\baz.txt");
        assert_eq!(&data, b"foo.slf");
        let data = read_file_data(&vfs, "foo\\bar\\baz.txt");
        assert_eq!(&data, b"foo.slf");
        let data = read_file_data(&vfs, "foo\\bar/baz.txt");
        assert_eq!(&data, b"foo.slf");
        let data = read_file_data(&vfs, "foo\\bar/ὀδυσσεύς.baz");
        assert_eq!(&data, b"foo.slf");

        temp.close().expect("close temp dir");
    }

    #[test]
    fn read_dir() {
        let (temp, dir, dir_fs) = create_temp_dir();
        create_foo_slf(&dir); // foo.slf
        create_foobar_slf(&dir); // foobar.slf
        create_file(&dir.join("foo/foo1.txt"));
        create_file(&dir.join("foo/Foo2.txt"));
        // Non-ascii paths
        create_file(&dir.join("foo/ὈΔΥΣΣΕΎΣ.txt"));
        create_file(&dir.join("foo/ХЦЧ"));
        create_file(&dir.join("foo/农历新年.txt"));

        let mut vfs = Vfs::new();
        vfs.add_dir(&dir).expect("dir");
        add_slf(&mut vfs, &dir_fs, "foo.slf");
        add_slf(&mut vfs, &dir_fs, "foobar.slf");

        let root_paths = ["foo", "foo.slf", "foobar.slf"];
        let foo_paths = [
            "农历新年.txt",
            "хцч",
            "ὀδυσσεύς.txt",
            "foo2.txt",
            "foo1.txt",
            "bar.txt",
            "bar",
        ];
        let foo_bar_paths = ["baz.txt", "ὀδυσσεύς.baz"];
        // case insensitive
        assert_vfs_read_dir(&vfs, "foo", &foo_paths);
        assert_vfs_read_dir(&vfs, "FOO", &foo_paths);
        assert_vfs_read_dir(&vfs, "foo/bar", &foo_bar_paths);
        assert_vfs_read_dir(&vfs, "FOO/Bar", &foo_bar_paths);

        // trailing path separators should not matter
        assert_vfs_read_dir(&vfs, "foo/", &foo_paths);
        assert_vfs_read_dir(&vfs, "FOO/", &foo_paths);
        assert_vfs_read_dir(&vfs, "foo/bar/", &foo_bar_paths);
        assert_vfs_read_dir(&vfs, "FOO/Bar/", &foo_bar_paths);

        // the kind of path separator should not matter
        assert_vfs_read_dir(&vfs, "foo\\bar", &foo_bar_paths);
        assert_vfs_read_dir(&vfs, "FOO\\Bar", &foo_bar_paths);
        assert_vfs_read_dir(&vfs, "foo/bar\\", &foo_bar_paths);
        assert_vfs_read_dir(&vfs, "FOO/Bar\\", &foo_bar_paths);

        // it should be possible to list root paths
        assert_vfs_read_dir(&vfs, "", &root_paths);
        assert_vfs_read_dir(&vfs, "/", &root_paths);

        let mut vfs = Vfs::new();
        add_slf(&mut vfs, &dir_fs, "foobar.slf");

        // it should be possible to list root paths in vfs just consisting of slf
        let root_paths = ["foo"];
        assert_vfs_read_dir(&vfs, "", &root_paths);
        assert_vfs_read_dir(&vfs, "/", &root_paths);

        // it should be possible to list paths that only exist in an slf prefix
        let foo_paths = ["bar"];
        assert_vfs_read_dir(&vfs, "foo", &foo_paths);
        assert_vfs_read_dir(&vfs, "foo/", &foo_paths);

        temp.close().expect("close temp dir");
    }

    // end of vfs tests
    //------------------

    use std::collections::BTreeSet;
    use std::io::{Read, Seek, SeekFrom, Write};
    use std::iter::FromIterator;
    use std::path::{Path, PathBuf};
    use std::sync::Arc;

    use serde_json::{json, Value};
    use stracciatella::file_formats::slf::{SlfEntry, SlfEntryState, SlfHeader};
    use stracciatella::fs;
    use stracciatella::fs::{OpenOptions, TempDir};
    use stracciatella::unicode::Nfc;
    use stracciatella::vfs::dir::DirFs;
    use stracciatella::vfs::{Vfs, VfsLayer};

    fn read_file_data(vfs: &Vfs, path: &str) -> Vec<u8> {
        let mut file = vfs.open(&Nfc::caseless_path(path)).expect("open");
        let mut data = Vec::new();
        file.read_to_end(&mut data).expect("read_to_end failed");
        data
    }

    fn read_file_data_in_layer(
        vfs: &Vfs,
        layer_index: usize,
        path: &str,
    ) -> std::io::Result<Option<Vec<u8>>> {
        match vfs.open_in_layer(layer_index, &Nfc::caseless_path(path)) {
            Ok(mut file) => {
                let mut data = Vec::new();
                file.read_to_end(&mut data).expect("read_to_end failed");
                Ok(Some(data))
            }
            Err(e) if e.kind() == std::io::ErrorKind::NotFound => Ok(None),
            Err(e) => Err(e),
        }
    }

    fn assert_vfs_read_dir(vfs: &Vfs, path: &str, expected: &[&str]) {
        let result = vfs.read_dir(&Nfc::caseless_path(path)).expect("read_dir");
        let expected = BTreeSet::from_iter(expected.iter().map(|s| Nfc::caseless_path(s)));
        assert_eq!(result, expected);
    }

    /// The inner file data is the name.
    fn create_slf(dir: &Path, name: &str, library_path: &str, entry_paths: &[&str]) -> PathBuf {
        let header = SlfHeader {
            library_name: name.to_owned(),
            library_path: library_path.to_owned(),
            num_entries: entry_paths.len() as i32,
            ok_entries: entry_paths.len() as i32,
            sort: 0xFFFF,
            version: 0x200,
            contains_subdirectories: if library_path.is_empty() { 0 } else { 1 },
        };
        let path = dir.join(&name);
        let mut file = OpenOptions::new()
            .write(true)
            .create_new(true)
            .open(&path)
            .expect("open new file for writing");
        header.to_output(&mut file).expect("write header");
        let mut entries = entry_paths
            .iter()
            .map(|&entry_path| {
                let offset = file.seek(SeekFrom::Current(0)).expect("seek to entry data");
                let data = name.as_bytes();
                file.write_all(data).expect("write entry data");
                SlfEntry {
                    file_path: entry_path.to_owned(),
                    offset: offset as u32,
                    length: data.len() as u32,
                    state: SlfEntryState::Ok,
                    file_time: 0,
                }
            })
            .collect::<Vec<SlfEntry>>();
        entries.sort_by(|a, b| a.file_path.cmp(&b.file_path));
        header
            .entries_to_output(&mut file, &entries)
            .expect("write entries");
        file.sync_all().expect("sync_all");
        path
    }

    /// The file data is the same as the filename.
    fn create_file(path: &Path) {
        let name = path
            .file_name()
            .expect("file name")
            .to_str()
            .expect("file name string");
        create_file_with_content(path, name.as_bytes());
    }

    /// Create file with json
    fn create_json_file(path: &Path, content: &Value) {
        let content = serde_json::to_vec(content).expect("json serialization");
        create_file_with_content(path, &content);
    }

    /// Create file with specific content
    fn create_file_with_content(path: &Path, content: &[u8]) {
        let dir = path.parent().expect("parent path");
        if !dir.exists() {
            fs::create_dir_all(&dir).expect("create_dir_all");
        }
        fs::write(path, content).expect("write");
    }

    /// The inner file data is "foobar.slf".
    fn create_foobar_slf(dir: &Path) {
        create_slf(dir, "foobar.slf", r"foo\bar\", &["baz.txt"]);
    }

    /// The inner file data is "foo.slf".
    fn create_foo_slf(dir: &Path) {
        create_slf(
            dir,
            "foo.slf",
            "foo\\",
            &["bar.txt", "bar\\baz.txt", "bar\\ὈΔΥΣΣΕΎΣ.baz"],
        );
    }

    /// The inner file data "data.slf".
    fn create_data_slf(dir: &Path) {
        create_slf(dir, "data.slf", "", &["foo.txt"]);
    }

    /// Add an slf to vfs
    fn add_slf(vfs: &mut Vfs, dir_fs: &Arc<dyn VfsLayer>, name: &str) {
        vfs.add_slf(dir_fs.open(&name.into()).expect("DirFs::open"))
            .expect("add_slf");
    }

    /// The temporary dir and it's contents are removed when TempDir is closed or goes out of scope.
    fn create_temp_dir() -> (TempDir, PathBuf, Arc<dyn VfsLayer>) {
        let temp = TempDir::new().expect("TempDir");
        let dir = temp.path().to_owned();
        let dir_fs = DirFs::new(&dir).expect("DirFs");
        (temp, dir, dir_fs)
    }
}
