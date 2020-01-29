//! This file contains tests for the `stracciatella::unicode` module.

// Nfc tests:
//  * 1 from the [`caseless`] crate,
//  * 7 from the "Unicode Normalization Forms" annex [(tr15)].
//
// [`caseless`]: https://github.com/unicode-rs/rust-caseless/blob/master/src/lib.rs
// [(tr15)]: http://www.unicode.org/reports/tr15
mod nfc {
    use stracciatella::unicode::Nfc;

    macro_rules! nfc_eq {
        ($left: expr, $right: expr) => {
            assert_eq!(Nfc::from($left), Nfc::from($right));
        };
    }

    macro_rules! nfc {
        ($input: expr, $expected: expr) => {
            assert_eq!(Nfc::from($input).as_str(), $expected);
        };
    }

    macro_rules! nfc_add {
        ($left: expr, $right: expr, $expected: expr) => {
            assert_eq!((Nfc::from($left) + $right).as_str(), $expected);
        };
    }

    #[test]
    fn caseless_test_strs() {
        assert_eq!(Nfc::caseless("Test Case").as_str(), "test case");
        assert_eq!(Nfc::caseless("Teſt Caſe").as_str(), "test case");
        assert_eq!(Nfc::caseless("spiﬃest").as_str(), "spiffiest");
        assert_eq!(Nfc::caseless("straße").as_str(), "strasse");
    }

    // tr15: Figure 1. Examples of Canonical Equivalence
    #[test]
    fn tr15_examples_of_canonical_equivalence() {
        nfc_eq!("\u{00C7}", "\u{0043}\u{0327}");
        nfc_eq!("\u{0071}\u{0307}\u{0323}", "\u{0071}\u{0323}\u{0307}");
        nfc_eq!("\u{AC00}", "\u{1100}\u{1161}");

        // NOTE this had U+03A9 GREEK CAPITAL LETTER OMEGA on both sides
        //      I replaced one with U+2126 OHM SIGN
        nfc_eq!("\u{03A9}", "\u{2126}");
    }

    // tr15: Figure 3. Singletons
    #[test]
    fn tr15_singletons() {
        nfc!("\u{212B}", "\u{00C5}");
        nfc!("\u{2126}", "\u{03A9}");
    }

    // tr15: Figure 4. Canonical Composites
    #[test]
    fn tr15_canonical_composites() {
        nfc!("\u{00C5}", "\u{00C5}");
        nfc!("\u{00F4}", "\u{00F4}");
    }

    // tr15: Figure 5. Multiple Combining Marks
    #[test]
    fn tr15_multiple_combining_marks() {
        nfc!("\u{1E69}", "\u{1E69}");
        nfc!("\u{1E0B}\u{0323}", "\u{1E0D}\u{0307}");
        nfc!("\u{0071}\u{0307}\u{0323}", "\u{0071}\u{0323}\u{0307}");
    }

    // tr15: Table 2. String Concatenation
    #[test]
    fn tr15_string_concatenation() {
        nfc_add!("\u{0061}", "\u{0302}", "\u{00E2}");
        nfc_add!("\u{1100}", "\u{1161}\u{11A8}", "\u{AC01}");
    }

    // tr15: Table 6. Basic Examples
    #[test]
    fn tr15_basic_examples() {
        nfc!("\u{1E0A}", "\u{1E0A}"); // a: D-dot_above => D-dot_above
        nfc!("\u{0044}\u{0307}", "\u{1E0A}"); // b: D + dot_above => D-dot_above
        nfc!("\u{1E0C}\u{0307}", "\u{1E0C}\u{0307}"); // c: D-dot_below + dot_above => D-dot_below + dot_above
        nfc!("\u{1E0A}\u{0323}", "\u{1E0C}\u{0307}"); // d: D-dot_above + dot_below => D-dot_below + dot_above
        nfc!("\u{0044}\u{0307}\u{0323}", "\u{1E0C}\u{0307}"); // e: D + dot_above + dot_below => D-dot_below + dot_above
        nfc!(
            "\u{0044}\u{0307}\u{031B}\u{0323}",
            "\u{1E0C}\u{031B}\u{0307}"
        ); // f: D + dot_above + horn + dot_below => D-dot_below + horn + dot_above
        nfc!("\u{1E16}", "\u{1E16}"); // f: E-macron-grave => E-macron-grave
        nfc!("\u{0112}\u{0301}", "\u{1E16}"); // h: E-macron + grave => E-macron-grave
        nfc!("\u{00C8}\u{0304}", "\u{00C8}\u{0304}"); // i: E-grave + macron => E-grave + macron
        nfc!("\u{212B}", "\u{00C5}"); // j: angstrom_sign => A-ring
        nfc!("\u{00C5}", "\u{00C5}"); // k: A-ring => A-ring
    }

    // tr15: Table 7. NFD and NFC Applied to Compatibility-Equivalent Strings
    #[test]
    fn tr15_nfc_applied_to_compatibility_equivalent_string() {
        nfc!("Äffin", "Äffin"); // l
        nfc!("Ä\u{FB03}n", "Ä\u{FB03}n"); // m
        nfc!("Henry IV", "Henry IV"); // n
        nfc!("Henry \u{2163}", "Henry \u{2163}"); // o
        nfc!("\u{30AC}", "\u{30AC}"); // p: ga => ga
        nfc!("\u{30AB}\u{3099}", "\u{30AC}"); // q: ka + ten => ga
        nfc!("\u{FF76}\u{FF9E}", "\u{FF76}\u{FF9E}"); // r: hw_ka + hw_ten => hw_ka + hw_ten
        nfc!("\u{30AB}\u{FF9E}", "\u{30AB}\u{FF9E}"); // s: ka + hw_ten => ka + hw_ten
        nfc!("\u{FF76}\u{3099}", "\u{FF76}\u{3099}"); // t: hw_ka + ten => hw_ka + ten

        // NOTE I'm not sure, but this one is probably U+CE8C HANGUL SYLLABLE KAK
        nfc!("\u{CE8C}", "\u{CE8C}"); // u: kaks => kaks
    }
}
