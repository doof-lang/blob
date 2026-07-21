export { EncodingError, Endian, TextEncoding } from "./types"

export import class BlobBuilder from "native_blob.hpp" as doof_blob::NativeBlobBuilder {
  isolated static constructor(size: long = 0L, endianness: Endian = .LittleEndian): BlobBuilder
  isolated getPosition(): long
  isolated setPosition(position: long): none
  isolated length(): long
  isolated writeZeroes(length: long): none
  isolated align(width: long): none
  isolated writeByte(value: byte): none
  isolated writeSignedByte(value: int): none
  isolated writeBool(value: bool): none
  isolated writeShort(value: int): none
  isolated writeUnsignedShort(value: int): none
  isolated writeInt(value: int): none
  isolated writeUnsignedInt(value: long): none
  isolated writeLong(value: long): none
  isolated writeFloat(value: float): none
  isolated writeDouble(value: double): none
  isolated writeBytes(value: readonly byte[]): none
  isolated writeString(value: string): none
  isolated writeText(value: string, encoding: TextEncoding = .Utf8): Result<int, EncodingError>
  isolated writeTextLossy(value: string, encoding: TextEncoding = .Utf8): int
  isolated build(): readonly byte[]
}

export import class BlobReader from "native_blob.hpp" as doof_blob::NativeBlobReader {
  data: readonly byte[]
  isolated static constructor(data: readonly byte[], endianness: Endian = .LittleEndian): BlobReader
  isolated getPosition(): long
  isolated setPosition(position: long): none
  isolated length(): long
  isolated remaining(): long
  isolated peekByte(): byte
  isolated skip(length: long): none
  isolated align(width: long): none
  isolated readByte(): byte
  isolated readSignedByte(): int
  isolated readBool(): bool
  isolated readShort(): int
  isolated readUnsignedShort(): int
  isolated readInt(): int
  isolated readUnsignedInt(): long
  isolated readLong(): long
  isolated readFloat(): float
  isolated readDouble(): double
  isolated readBytes(length: long): readonly byte[]
  isolated readString(length: long): string
  isolated readText(length: long, encoding: TextEncoding = .Utf8): Result<string, EncodingError>
  isolated readTextLossy(length: long, encoding: TextEncoding = .Utf8): string
  isolated findNextAny(candidates: readonly byte[]): long | none
}

export function decodeUtf8(data: readonly byte[]): Result<string, EncodingError> {
  reader := BlobReader(data)
  return reader.readText(data.length, .Utf8)
}
