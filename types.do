export enum Endian {
  BigEndian = 0,
  LittleEndian = 1,
}

export enum TextEncoding {
  Utf8 = 0,
  Utf16LE = 1,
  Utf16BE = 2,
  Latin1 = 3,
  Windows1252 = 4,
  CP437 = 5,
  Ascii = 6,
}

export enum EncodingError {
  InvalidData = 0,
  UnrepresentableCharacter = 1,
  OutputTooLarge = 2,
}
