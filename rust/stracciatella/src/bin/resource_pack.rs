//! This file contains the code for the resource-pack executable.
//!
//! A resource pack is a json file that identifies the resources in a particular game version/mod.
//!
//!
//! # Create a resource pack:
//!
//! Example:
//! ```
//! resource-pack create --name "My resource pack" --pretty --file-size --hash md5 --gamedir /path/to/game/dir --property vanilla_version ENGLISH --output pack.json
//! ```
//!

use std::fmt::Debug;
use std::fs;
use std::path::Path;
use std::process;

use clap::{crate_version, App, Arg, ArgMatches, SubCommand};
use serde_json;

use stracciatella::fs::resolve_existing_components;
use stracciatella::res::{ResourcePackBuilder, ResourcePropertiesExt};
use stracciatella::unicode::Nfc;

/// Entry point of the resource-pack executable.
fn main() {
    let cmd_create = SubCommand::with_name("create")
        .about("Creates a resource pack.")
        .version("1.0")
        .arg(
            Arg::with_name("file-size")
                .help("Adds the file size to the resource properties")
                .long("file-size"),
        )
        .arg(
            Arg::with_name("pretty")
                .help("Outputs with the pretty formatter")
                .long("pretty"),
        )
        .arg(
            Arg::with_name("archive")
                .help("Adds the archive contents as resources")
                .long("archive")
                .value_name("TYPE")
                .possible_values(&["slf"])
                .takes_value(true)
                .multiple(true),
        )
        .arg(
            Arg::with_name("gamedir")
                .help("Adds the PATH/data directory (case-insensitive) as resources, PATH/data/temp is ignored")
                .long("gamedir")
                .value_name("PATH")
                .takes_value(true)
                .multiple(true),
        )
        .arg(
            Arg::with_name("hash")
                .help("Adds the hash of the file data to the resource properties")
                .long("hash")
                .value_name("TYPE")
                .possible_values(&["md5"])
                .takes_value(true)
                .multiple(true),
        )
        .arg(
            Arg::with_name("name")
                .help("Name of the resource pack")
                .long("name")
                .value_name("NAME")
                .takes_value(true)
                .required(true),
        )
        .arg(
            Arg::with_name("output")
                .help("Outputs to file instead of stdout")
                .long("output")
                .value_name("PATH")
                .takes_value(true),
        )
        .arg(
            Arg::with_name("property")
                .help("Sets a new string property after the resource pack is created")
                .long("property")
                .value_name("NAME")
                .value_name("VALUE")
                .takes_value(true)
                .multiple(true),
        );

    let matches = App::new("resource-pack")
        .about("Tool that creates resource packs.")
        .version(crate_version!())
        .subcommand(cmd_create)
        .get_matches();

    if let Some(matches) = matches.subcommand_matches("create") {
        subcommand_create(matches);
    }
}

/// Creates a resource pack.
fn subcommand_create(matches: &ArgMatches) {
    let mut builder = ResourcePackBuilder::new();

    if matches.is_present("file-size") {
        builder.with_file_size();
    }

    if let Some(values) = matches.values_of("hash") {
        for hash in values {
            builder.with_hash(hash);
        }
    }

    if let Some(values) = matches.values_of("archive") {
        for archive in values {
            builder.with_archive(archive);
        }
    }

    if let Some(values) = matches.values_of_os("gamedir") {
        for gamedir in values {
            let path =
                resolve_existing_components(Path::new("data"), Some(Path::new(gamedir)), true);
            if path.exists() {
                builder.with_path(&path, &path);
            } else {
                graceful_error(&format!("Data dir not found in {:?}", gamedir));
            }
        }
    }

    let mut pack = graceful_unwrap(
        "Creating",
        builder.execute(matches.value_of("name").unwrap()),
    );

    if let Some(mut iter) = matches.values_of("property") {
        while let Some(prop) = iter.next() {
            let value = iter.next().unwrap();
            if pack.get_property(prop).is_some() {
                graceful_error(&format!("Property '{}' already exists", prop));
            }
            pack.set_property(prop, value);
        }
    }

    let json: Nfc = graceful_unwrap(
        "Serializing to json",
        if matches.is_present("pretty") {
            serde_json::to_string_pretty(&pack)
        } else {
            serde_json::to_string(&pack)
        },
    )
    .into();

    if let Some(path) = matches.value_of_os("output") {
        graceful_unwrap("Writing to output", fs::write(path, json.as_str()));
    } else {
        println!("{}", json);
    }
}

/// Either unwraps a result or prints an error to stderr and exits with 1.
fn graceful_unwrap<T, E: Debug>(desc: &str, result: Result<T, E>) -> T {
    match result {
        Ok(value) => value,
        Err(err) => {
            eprintln!("{}: {:?}", desc, err);
            process::exit(1);
        }
    }
}

/// Prints an error to stderr and exits with 1.
fn graceful_error(desc: &str) {
    eprintln!("{}", desc);
    process::exit(1);
}
