use std::collections::BTreeMap;

use serde_json::Value;
use stracciatella::json::de;

use super::{common::*, vec::VecCString};

#[derive(Debug, Clone)]
pub struct RJsonValue(pub Value);

impl RJsonValue {
    pub fn from_value(value: Value) -> Self {
        RJsonValue(value)
    }

    fn deserialize(value: &str) -> Result<Self, String> {
        let value = de::from_string(value)?;

        Ok(RJsonValue(value))
    }

    fn serialize(&self) -> Result<String, String> {
        serde_json::ser::to_string(&self.0).map_err(|e| e.to_string())
    }

    fn serialize_pretty(&self) -> Result<String, String> {
        serde_json::ser::to_string_pretty(&self.0).map_err(|e| e.to_string())
    }

    fn to_array(&self) -> Result<RJsonArray, String> {
        self.0
            .as_array()
            .map(|arr| RJsonArray(arr.iter().map(|v| RJsonValue(v.clone())).collect()))
            .ok_or_else(|| "expected array".to_string())
    }

    fn is_array(&self) -> bool {
        self.0.is_array()
    }

    fn to_object(&self) -> Result<RJsonObject, String> {
        self.0
            .as_object()
            .map(|obj| {
                RJsonObject(
                    obj.iter()
                        .map(|(k, v)| (k.clone(), RJsonValue(v.clone())))
                        .collect(),
                )
            })
            .ok_or_else(|| "expected object".to_string())
    }

    fn is_object(&self) -> bool {
        self.0.is_object()
    }

    fn to_i64(&self) -> Result<i64, String> {
        self.0
            .as_i64()
            .ok_or_else(|| "expected integer".to_string())
    }

    fn is_i64(&self) -> bool {
        self.0.is_i64()
    }

    fn to_f64(&self) -> Result<f64, String> {
        self.0.as_f64().ok_or_else(|| "expected float".to_string())
    }

    fn is_f64(&self) -> bool {
        self.0.is_f64()
    }

    fn to_bool(&self) -> Result<bool, String> {
        self.0
            .as_bool()
            .ok_or_else(|| "expected boolean".to_string())
    }

    fn is_bool(&self) -> bool {
        self.0.is_boolean()
    }

    fn to_string(&self) -> Result<String, String> {
        self.0
            .as_str()
            .map(|s| s.to_string())
            .ok_or_else(|| "expected string".to_string())
    }

    fn is_string(&self) -> bool {
        self.0.is_string()
    }
}

impl<T> From<T> for RJsonValue
where
    T: Into<Value>,
{
    fn from(v: T) -> Self {
        RJsonValue(v.into())
    }
}

#[derive(Debug, Clone, Default)]
pub struct RJsonArray(Vec<RJsonValue>);

impl RJsonArray {
    fn len(&self) -> usize {
        self.0.len()
    }

    fn get(&self, idx: usize) -> Option<RJsonValue> {
        self.0.get(idx).cloned()
    }

    fn push(&mut self, val: RJsonValue) {
        self.0.push(val)
    }

    fn to_value(&self) -> RJsonValue {
        RJsonValue(Value::Array(self.0.iter().map(|v| v.0.clone()).collect()))
    }
}

#[derive(Debug, Clone, Default)]
pub struct RJsonObject(BTreeMap<String, RJsonValue>);

impl RJsonObject {
    fn get(&self, prop: &str) -> Option<RJsonValue> {
        self.0.get(prop).cloned()
    }

    fn set(&mut self, prop: &str, value: &RJsonValue) {
        self.0.insert(prop.to_string(), value.clone());
    }

    fn keys(&self) -> VecCString {
        let keys: Vec<_> = self.0.keys().map(|v| c_string_from_str(v)).collect();
        keys.into()
    }

    fn contains_key(&self, prop: &str) -> bool {
        self.0.contains_key(prop)
    }

    fn to_value(&self) -> RJsonValue {
        RJsonValue(Value::Object(
            self.0
                .iter()
                .map(|(k, v)| (k.clone(), v.0.clone()))
                .collect(),
        ))
    }
}

#[no_mangle]
pub extern "C" fn RJsonValue_deserialize(value: *const c_char) -> *mut RJsonValue {
    forget_rust_error();
    let value = str_from_c_str_or_panic(unsafe_c_str(value));
    match RJsonValue::deserialize(value) {
        Ok(value) => into_ptr(value),
        Err(e) => {
            remember_rust_error(&e);
            std::ptr::null_mut()
        }
    }
}

#[no_mangle]
pub extern "C" fn RJsonValue_fromString(value: *const c_char) -> *mut RJsonValue {
    let value = str_from_c_str_or_panic(unsafe_c_str(value)).to_string();
    into_ptr(value.into())
}

#[no_mangle]
pub extern "C" fn RJsonValue_fromInt(value: i32) -> *mut RJsonValue {
    into_ptr(value.into())
}

#[no_mangle]
pub extern "C" fn RJsonValue_fromUInt(value: u32) -> *mut RJsonValue {
    into_ptr(value.into())
}

#[no_mangle]
pub extern "C" fn RJsonValue_fromDouble(value: f64) -> *mut RJsonValue {
    into_ptr(value.into())
}

#[no_mangle]
pub extern "C" fn RJsonValue_fromBool(value: bool) -> *mut RJsonValue {
    into_ptr(value.into())
}

/// Destroys the JsonValue instance.
/// coverity[+free : arg-0]
#[no_mangle]
pub extern "C" fn RJsonValue_destroy(value: *mut RJsonValue) {
    let _drop_me = from_ptr(value);
}

/// Converts the JsonValue if possible.
/// Sets success to false if it cannot be converted.
#[no_mangle]
pub extern "C" fn RJsonValue_toArray(value: *const RJsonValue) -> *mut RJsonArray {
    let value = unsafe_ref(value);
    match value.to_array() {
        Ok(array) => into_ptr(array),
        Err(e) => {
            remember_rust_error(e);
            std::ptr::null_mut()
        }
    }
}

/// Converts the JsonValue if possible.
/// Sets success to false if it cannot be converted.
#[no_mangle]
pub extern "C" fn RJsonValue_toObject(value: *const RJsonValue) -> *mut RJsonObject {
    let value = unsafe_ref(value);
    match value.to_object() {
        Ok(obj) => into_ptr(obj),
        Err(e) => {
            remember_rust_error(e);
            std::ptr::null_mut()
        }
    }
}

/// Converts the JsonValue if possible.
/// Sets success to false if it cannot be converted.
#[no_mangle]
pub extern "C" fn RJsonValue_toString(value: *const RJsonValue) -> *mut c_char {
    let value = unsafe_ref(value);
    match value.to_string() {
        Ok(obj) => c_string_from_str(&obj).into_raw(),
        Err(e) => {
            remember_rust_error(e);
            std::ptr::null_mut()
        }
    }
}

macro_rules! is_type {
    ($value:expr, $fn:ident) => {{
        let value = unsafe_ref($value);
        value.$fn()
    }};
}

macro_rules! to_primitive {
    ($value:expr, $success:expr, $fn:ident) => {{
        let value = unsafe_ref($value);
        let success = unsafe_mut($success);
        match value.$fn() {
            Ok(v) => {
                *success = true;
                v
            }
            Err(e) => {
                remember_rust_error(e);
                Default::default()
            }
        }
    }};
}

/// Converts the JsonValue if possible.
/// Sets success to false if it cannot be converted.
#[no_mangle]
pub extern "C" fn RJsonValue_toInt64(value: *const RJsonValue, success: *mut bool) -> i64 {
    to_primitive!(value, success, to_i64)
}

/// Converts the JsonValue if possible.
/// Sets success to false if it cannot be converted.
#[no_mangle]
pub extern "C" fn RJsonValue_toDouble(value: *const RJsonValue, success: *mut bool) -> f64 {
    to_primitive!(value, success, to_f64)
}

/// Converts the JsonValue if possible.
/// Sets success to false if it cannot be converted.
#[no_mangle]
pub extern "C" fn RJsonValue_toBool(value: *const RJsonValue, success: *mut bool) -> bool {
    to_primitive!(value, success, to_bool)
}

/// Checks if the value is an array
#[no_mangle]
pub extern "C" fn RJsonValue_isArray(value: *const RJsonValue) -> bool {
    is_type!(value, is_array)
}

/// Checks if the value is an object
#[no_mangle]
pub extern "C" fn RJsonValue_isObject(value: *const RJsonValue) -> bool {
    is_type!(value, is_object)
}

/// Checks if the value is a string
#[no_mangle]
pub extern "C" fn RJsonValue_isString(value: *const RJsonValue) -> bool {
    is_type!(value, is_string)
}

/// Checks if the value is an int
#[no_mangle]
pub extern "C" fn RJsonValue_isInt(value: *const RJsonValue) -> bool {
    is_type!(value, is_i64)
}

/// Checks if the value is a double
#[no_mangle]
pub extern "C" fn RJsonValue_isDouble(value: *const RJsonValue) -> bool {
    is_type!(value, is_f64)
}

/// Checks if the value is a bool
#[no_mangle]
pub extern "C" fn RJsonValue_isBool(value: *const RJsonValue) -> bool {
    is_type!(value, is_bool)
}

/// Serializes the JSON value to a string.
#[no_mangle]
pub extern "C" fn RJsonValue_serialize(value: *const RJsonValue, pretty: bool) -> *mut c_char {
    let value = unsafe_ref(value);
    let result = if pretty {
        value.serialize_pretty()
    } else {
        value.serialize()
    };
    match result {
        Ok(v) => c_string_from_str(&v).into_raw(),
        Err(e) => {
            remember_rust_error(e);
            std::ptr::null_mut()
        }
    }
}

#[no_mangle]
pub extern "C" fn RJsonArray_new() -> *mut RJsonArray {
    into_ptr(RJsonArray(Default::default()))
}

/// Destroys the JsonArray instance.
/// coverity[+free : arg-0]
#[no_mangle]
pub extern "C" fn RJsonArray_destroy(array: *mut RJsonArray) {
    let _drop_me = from_ptr(array);
}

#[no_mangle]
pub extern "C" fn RJsonArray_length(array: *const RJsonArray) -> usize {
    let array = unsafe_ref(array);
    array.len()
}

#[no_mangle]
pub extern "C" fn RJsonArray_get(array: *const RJsonArray, idx: usize) -> *mut RJsonValue {
    let array = unsafe_ref(array);
    match array
        .get(idx)
        .ok_or_else(|| format!("failed to get element at index {}", idx))
    {
        Ok(val) => into_ptr(val),
        Err(e) => {
            remember_rust_error(e);
            std::ptr::null_mut()
        }
    }
}

#[no_mangle]
pub extern "C" fn RJsonArray_push(array: *mut RJsonArray, value: *const RJsonValue) {
    let array = unsafe_mut(array);
    let value = unsafe_ref(value);
    array.push(value.clone())
}

#[no_mangle]
pub extern "C" fn RJsonArray_toValue(array: *const RJsonArray) -> *mut RJsonValue {
    let arr = unsafe_ref(array);
    into_ptr(arr.to_value())
}

#[no_mangle]
pub extern "C" fn RJsonObject_new() -> *mut RJsonObject {
    into_ptr(RJsonObject(Default::default()))
}

/// Destroys the JsonObject instance.
/// coverity[+free : arg-0]
#[no_mangle]
pub extern "C" fn RJsonObject_destroy(obj: *mut RJsonObject) {
    let _drop_me = from_ptr(obj);
}

#[no_mangle]
pub extern "C" fn RJsonObject_get(obj: *const RJsonObject, prop: *const c_char) -> *mut RJsonValue {
    let obj = unsafe_ref(obj);
    let prop = str_from_c_str_or_panic(unsafe_c_str(prop));
    match obj
        .get(prop)
        .ok_or_else(|| format!("failed to get property {}", prop))
    {
        Ok(val) => into_ptr(val),
        Err(e) => {
            remember_rust_error(e);
            std::ptr::null_mut()
        }
    }
}

#[no_mangle]
pub extern "C" fn RJsonObject_set(
    obj: *mut RJsonObject,
    prop: *const c_char,
    value: *const RJsonValue,
) {
    let obj = unsafe_mut(obj);
    let prop = str_from_c_str_or_panic(unsafe_c_str(prop));
    let value = unsafe_ref(value);

    obj.set(prop, value);
}

#[no_mangle]
pub extern "C" fn RJsonObject_has(obj: *const RJsonObject, prop: *const c_char) -> bool {
    let obj = unsafe_ref(obj);
    let prop = str_from_c_str_or_panic(unsafe_c_str(prop));
    obj.contains_key(prop)
}

#[no_mangle]
pub extern "C" fn RJsonObject_keys(obj: *const RJsonObject) -> *mut VecCString {
    let obj = unsafe_ref(obj);
    into_ptr(obj.keys())
}

#[no_mangle]
pub extern "C" fn RJsonObject_toValue(obj: *const RJsonObject) -> *mut RJsonValue {
    let obj = unsafe_ref(obj);
    into_ptr(obj.to_value())
}
