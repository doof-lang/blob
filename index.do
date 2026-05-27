export { EncodingError, Endian, TextEncoding } from "./types"

export import class BlobBuilder from "native_blob.hpp" as doof_blob::NativeBlobBuilder {
  static constructor(size: long = 0L, endianness: Endian = .LittleEndian): BlobBuilder
  getPosition(): long
  setPosition(position: long): void
  length(): long
  writeZeroes(length: long): void
  align(width: long): void
  writeByte(value: byte): void
  writeSignedByte(value: int): void
  writeBool(value: bool): void
  writeShort(value: int): void
  writeUnsignedShort(value: int): void
  writeInt(value: int): void
  writeUnsignedInt(value: long): void
  writeLong(value: long): void
  writeFloat(value: float): void
  writeDouble(value: double): void
  writeBytes(value: readonly byte[]): void
  writeString(value: string): void
  writeText(value: string, encoding: TextEncoding = .Utf8): Result<int, EncodingError>
  writeTextLossy(value: string, encoding: TextEncoding = .Utf8): int
  build(): readonly byte[]
}

export import class BlobReader from "native_blob.hpp" as doof_blob::NativeBlobReader {
  data: readonly byte[]
  static constructor(data: readonly byte[], endianness: Endian = .LittleEndian): BlobReader
  getPosition(): long
  setPosition(position: long): void
  length(): long
  remaining(): long
  peekByte(): byte
  skip(length: long): void
  align(width: long): void
  readByte(): byte
  readSignedByte(): int
  readBool(): bool
  readShort(): int
  readUnsignedShort(): int
  readInt(): int
  readUnsignedInt(): long
  readLong(): long
  readFloat(): float
  readDouble(): double
  readBytes(length: long): readonly byte[]
  readString(length: long): string
  readText(length: long, encoding: TextEncoding = .Utf8): Result<string, EncodingError>
  readTextLossy(length: long, encoding: TextEncoding = .Utf8): string
  findNextAny(candidates: readonly byte[]): long | null
}
