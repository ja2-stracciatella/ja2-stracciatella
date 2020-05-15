/// Performs a checked unsigned signed addition.
pub fn checked_add_u64_i64(a: u64, b: i64) -> Option<u64> {
    if b > 0 {
        a.checked_add(b as u64)
    } else if b < 0 {
        a.checked_sub(-b as u64)
    } else {
        Some(a)
    }
}
