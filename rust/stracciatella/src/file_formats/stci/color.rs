//! This module contains color related functionality for the STCI image format

use byteorder::{ReadBytesExt, WriteBytesExt, LE};
use std::io::{Read, Result, Write};

/// Mask used to get red bits from Rgb565 color
pub const STCI_RGB565_RED_MASK: u32 = 0xF800;
/// Mask used to get green bits from Rgb565 color
pub const STCI_RGB565_GREEN_MASK: u32 = 0x7E0;
/// Mask used to get blue bits from Rgb565 color
pub const STCI_RGB565_BLUE_MASK: u32 = 0x1F;

/// Rgb color representation with 8 bit per color
///
/// This is used in indexed STCI images as the palette colors
#[derive(Debug, Default, Clone, Copy, PartialEq)]
pub struct StciRgb888(pub u8, pub u8, pub u8);

impl StciRgb888 {
    /// Read a single Rgb888 pixel from input
    #[allow(dead_code)]
    pub fn from_input<T>(input: &mut T) -> Result<Self>
    where
        T: Read,
    {
        let red = input.read_u8()?;
        let green = input.read_u8()?;
        let blue = input.read_u8()?;

        Ok(Self(red, green, blue))
    }

    /// Write a single Rgb888 pixel to output
    #[allow(dead_code)]
    pub fn to_output<T>(self, output: &mut T) -> Result<()>
    where
        T: Write,
    {
        output.write_u8(self.0)?;
        output.write_u8(self.1)?;
        output.write_u8(self.2)?;
        Ok(())
    }
}

const fn max_value_565_shift_right(mask: u32, shift: u32) -> u32 {
    std::u32::MAX & mask >> shift
}

const fn max_value_565_shift_left(mask: u32, shift: u32) -> u32 {
    std::u32::MAX & mask << shift
}

impl From<StciRgb565> for StciRgb888 {
    /// This conversion is slightly different from the one used in the original source code
    /// It ensures that the whole 0..255 range of colors is used.
    fn from(value16: StciRgb565) -> Self {
        let r: u32 = (u32::from(value16.0) & STCI_RGB565_RED_MASK) >> 8;
        let g: u32 = (u32::from(value16.0) & STCI_RGB565_GREEN_MASK) >> 3;
        let b: u32 = (u32::from(value16.0) & STCI_RGB565_BLUE_MASK) << 3;
        let r = (255 * r) / max_value_565_shift_right(STCI_RGB565_RED_MASK, 8);
        let g = (255 * g) / max_value_565_shift_right(STCI_RGB565_GREEN_MASK, 3);
        let b = (255 * b) / max_value_565_shift_left(STCI_RGB565_BLUE_MASK, 3);
        StciRgb888(r as u8, g as u8, b as u8)
    }
}

/// Rgb color representation with 5 bits red, 6 bits green, 5 bits blue
///
/// This is used in rgb STCI images as pixel data
///
/// ```
/// use stracciatella::file_formats::stci::{StciRgb565, StciRgb888};
///
/// // White should be white
/// assert_eq!(StciRgb888::from(StciRgb565(std::u16::MAX)), StciRgb888(255, 255, 255));
/// // Black should be black
/// assert_eq!(StciRgb888::from(StciRgb565(0)), StciRgb888(0, 0, 0));
/// ```
#[derive(Debug, Default, Clone, Copy, PartialEq)]
pub struct StciRgb565(pub u16);

impl StciRgb565 {
    /// Read a single Rgb565 pixel from input
    #[allow(dead_code)]
    pub fn from_input<T>(input: &mut T) -> Result<Self>
    where
        T: Read,
    {
        let v = input.read_u16::<LE>()?;

        Ok(Self(v))
    }

    /// Write a single Rgb565 pixel to output
    #[allow(dead_code)]
    pub fn to_output<T>(self, output: &mut T) -> Result<()>
    where
        T: Write,
    {
        output.write_u16::<LE>(self.0)?;
        Ok(())
    }
}
