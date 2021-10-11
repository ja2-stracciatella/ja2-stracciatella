//! Serialize and desialize json with comments.
//!
//! Supported comment styles:
//! - C style block comments (`/* ... */`)
//! - C style line comments (`// ...`)
//! - Shell style line comments (`# ...`)
//!
//! Shell style line comments should be avoided since RapidJSON does not support them.
//!
//! # Links:
//! - [serde_json](https://crates.io/crates/serde_json)
//! - [json_comments](https://crates.io/crates/json_comments)
//! - [JSON](https://json.org/)
//! - [RapidJSON](http://rapidjson.org/) (used in C++ code)

pub mod de {
    //! Deserialize json with comments.
    //!
    //! Uses `StripComments` from the [`json_comments`] crate to skip the comments.
    //!
    //! [`json_comments`]: https://crates.io/crates/json_comments

    use json_comments::StripComments;
    use serde::de::DeserializeOwned;

    /// Converts a string containing json with comments to a value.
    pub fn from_string<T>(json: &str) -> Result<T, String>
    where
        T: DeserializeOwned,
    {
        let reader = StripComments::new(json.as_bytes());
        serde_json::from_reader(reader).map_err(|x| format!("{}", x))
    }
}

pub mod ser {
    //! Serialize json with comments.
    //!
    //! Uses `Value` from the [`serde_json`] crate as intermediate json.
    //!
    //! [`serde_json`]: https://crates.io/crates/serde_json

    use serde::Serialize;
    use serde_json::Value;

    use crate::json::skip;

    /// Converts a value to a string containing json.
    ///
    /// Produces "pretty" json with two space indentation.
    /// Does not terminate the last line with a `'\n'`.
    pub fn to_string<T>(value: &T) -> Result<String, String>
    where
        T: Serialize,
    {
        to_string_with_comments(value, |_| None)
    }

    /// Converts a value to a string containing json with comments.
    ///
    /// Produces "pretty" json with two space indentation and comments.
    /// Does not terminate the last line with a `'\n'`.
    ///
    /// Supports comments in the root and in each key path from the root.
    /// Objects inside arrays don't have comments.
    ///
    /// The `comments` closure receives `""` in the root and the key path with a dot separator (`"key1.key2"`) in each name/value pair.
    /// Dots in keys are not escaped, so they should be avoided.
    pub fn to_string_with_comments<T, F>(value: &T, comments: F) -> Result<String, String>
    where
        T: Serialize,
        F: Fn(&str) -> Option<String>,
    {
        let value = serde_json::to_value(value).map_err(|x| format!("{}", x))?;
        let mut serializer = Serializer::new(comments);
        serializer.serialize(&value)?;
        Ok(serializer.json)
    }

    struct Serializer<F> {
        json: String,
        path: String,
        depth: usize,
        comments: F,
    }

    impl<F: Fn(&str) -> Option<String>> Serializer<F> {
        fn new(comments: F) -> Self {
            Self {
                json: String::new(),
                path: String::new(),
                depth: 0,
                comments,
            }
        }

        fn serialize(&mut self, value: &Value) -> Result<(), String> {
            self.push_comments()?;
            self.push_prefix();
            self.push_value(&value)?;
            Ok(())
        }

        fn push(&mut self, c: char) {
            self.json.push(c);
        }

        fn push_str(&mut self, s: &str) {
            self.json.push_str(s);
        }

        fn push_prefix(&mut self) {
            if !self.json.is_empty() {
                self.push('\n');
            }
            for _ in 0..self.depth {
                self.push(' ');
            }
        }

        fn push_comments(&mut self) -> Result<(), String> {
            let path = &self.path;
            let result = (self.comments)(path);
            if let Some(comments) = result {
                if let Err(pos) = skip::comments(&comments) {
                    if pos == comments.len() {
                        return Err(format!(
                            "{}: incomplete comments {:?}",
                            &self.path, &comments
                        ));
                    }
                    return Err(format!(
                        "{}: invalid comments {:?}: error at position {}",
                        &self.path, &comments, &pos
                    ));
                }
                for line in comments.lines() {
                    self.push_prefix();
                    self.push_str(&line);
                }
            }
            Ok(())
        }

        fn push_value(&mut self, value: &Value) -> Result<(), String> {
            match value {
                Value::Object(map) => {
                    self.push('{');
                    self.depth += 2;
                    let len = self.path.len();
                    let mut first = true;
                    for (key, value) in map.iter() {
                        if !first {
                            self.push(',');
                        }
                        self.path_truncate(len);
                        self.path_push(&key);
                        let key = serde_json::to_string(key).map_err(|x| format!("{}", x))?;
                        self.push_comments()?;
                        self.push_prefix();
                        self.push_str(&key);
                        self.push_str(": ");
                        self.push_value(value)?;
                        first = false;
                    }
                    self.depth -= 2;
                    self.path_truncate(len);
                    self.push_prefix();
                    self.push('}');
                }
                value => {
                    let value = serde_json::to_string(value).map_err(|x| format!("{}", x))?;
                    self.push_str(&value);
                }
            }
            Ok(())
        }

        fn path_push(&mut self, key: &str) {
            if !self.path.is_empty() {
                self.path.push('.');
            }
            self.path.push_str(key);
        }

        fn path_truncate(&mut self, len: usize) {
            self.path.truncate(len);
        }
    }
}

pub mod skip {
    //! Skip over string data that has json with comments.

    /// Possible states while skipping whilespace and comments.
    #[derive(Eq, PartialEq, Copy, Clone)]
    enum CommentsSt {
        /// Whitespace.
        Space,
        /// Possible start of a C style comment.
        Start,
        /// Inside a line comment.
        Line,
        /// Inside a C style block comment.
        Block,
        /// Possible end of a C style block comment.
        End,
        /// Not whitespace and not comments.
        Error,
    }

    /// Skips over whitespace and comments.
    ///
    /// Returns `Ok(s.len())` if everything is whitespace and comments,
    /// `Err(s.len())` if it is still inside a comment,
    /// `Err(index)` if it found something else.
    pub fn comments(s: &str) -> Result<usize, usize> {
        let mut st = CommentsSt::Space;
        for (index, c) in s.char_indices() {
            st = match st {
                CommentsSt::Space => match c {
                    ' ' | '\t' | '\r' | '\n' => CommentsSt::Space,
                    '#' => CommentsSt::Line,
                    '/' => CommentsSt::Start,
                    _ => CommentsSt::Error,
                },
                CommentsSt::Start => match c {
                    '/' => CommentsSt::Line,
                    '*' => CommentsSt::Block,
                    _ => CommentsSt::Error,
                },
                CommentsSt::Line => match c {
                    '\n' => CommentsSt::Space,
                    _ => CommentsSt::Line,
                },
                CommentsSt::Block => match c {
                    '*' => CommentsSt::End,
                    _ => CommentsSt::Block,
                },
                CommentsSt::End => match c {
                    '/' => CommentsSt::Space,
                    '*' => CommentsSt::End,
                    _ => CommentsSt::Block,
                },
                _ => CommentsSt::Error,
            };
            if st == CommentsSt::Error {
                return Err(index); // not whitespace and not comments
            }
        }
        if st != CommentsSt::Space {
            Err(s.len()) // still inside a comment
        } else {
            Ok(s.len())
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::json::*;

    #[test]
    fn test_json_with_comments() {
        let data = serde_json::json!({
            "outer": {
                "inner": null
            },
            "key": "1",
            "arr": [2,{"3":4}]
        });
        let json_text = ser::to_string_with_comments(&data, |path| match path {
            "" => Some("// aaa\n".into()),
            "outer" => Some("/* bbb */".into()),
            "outer.inner" => Some("/* ccc\n   ddd */".into()),
            "key" => Some("# eee\n# fff\n".into()),
            "arr" => None,
            _ => panic!("unexpected path {}", path),
        })
        .expect("json text");
        assert_eq!(
            &json_text,
            r#"// aaa
{
  /* bbb */
  "outer": {
    /* ccc
       ddd */
    "inner": null
  },
  # eee
  # fff
  "key": "1",
  "arr": [2,{"3":4}]
}"#
        );
        let value: serde_json::Value = de::from_string(&json_text).expect("value");
        assert_eq!(&data, &value);
    }
}
