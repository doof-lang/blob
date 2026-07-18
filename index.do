export { EncodingError, Endian, TextEncoding } from "./types"

export import class BlobBuilder from "native_blob.hpp" as doof_blob::NativeBlobBuilder {
  isolated static constructor(size: long = 0L, endianness: Endian = .LittleEndian): BlobBuilder
  isolated getPosition(): long
  isolated setPosition(position: long): void
  isolated length(): long
  isolated writeZeroes(length: long): void
  isolated align(width: long): void
  isolated writeByte(value: byte): void
  isolated writeSignedByte(value: int): void
  isolated writeBool(value: bool): void
  isolated writeShort(value: int): void
  isolated writeUnsignedShort(value: int): void
  isolated writeInt(value: int): void
  isolated writeUnsignedInt(value: long): void
  isolated writeLong(value: long): void
  isolated writeFloat(value: float): void
  isolated writeDouble(value: double): void
  isolated writeBytes(value: readonly byte[]): void
  isolated writeString(value: string): void
  isolated writeText(value: string, encoding: TextEncoding = .Utf8): Result<int, EncodingError>
  isolated writeTextLossy(value: string, encoding: TextEncoding = .Utf8): int
  isolated build(): readonly byte[]
}

export import class BlobReader from "native_blob.hpp" as doof_blob::NativeBlobReader {
  data: readonly byte[]
  isolated static constructor(data: readonly byte[], endianness: Endian = .LittleEndian): BlobReader
  isolated getPosition(): long
  isolated setPosition(position: long): void
  isolated length(): long
  isolated remaining(): long
  isolated peekByte(): byte
  isolated skip(length: long): void
  isolated align(width: long): void
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
  isolated findNextAny(candidates: readonly byte[]): long | null
}

export function decodeUtf8(data: readonly byte[]): Result<string, EncodingError> {
  reader := BlobReader(data)
  return reader.readText(data.length, .Utf8)
}
