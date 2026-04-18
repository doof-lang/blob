export { Endian } from "./types"

export import class BlobBuilder from "native_blob.hpp" as doof_blob::NativeBlobBuilder {
  static create(size: long = 0L, endianness: Endian = .LittleEndian): BlobBuilder
  getPosition(): long
  setPosition(position: long): void
  length(): long
  writeByte(value: byte): void
  writeBool(value: bool): void
  writeInt(value: int): void
  writeLong(value: long): void
  writeFloat(value: float): void
  writeDouble(value: double): void
  writeBytes(value: readonly byte[]): void
  writeString(value: string): void
  build(): readonly byte[]
}

export import class BlobReader from "native_blob.hpp" as doof_blob::NativeBlobReader {
  static create(data: readonly byte[], endianness: Endian = .LittleEndian): BlobReader
  getPosition(): long
  setPosition(position: long): void
  length(): long
  remaining(): long
  readByte(): byte
  readBool(): bool
  readInt(): int
  readLong(): long
  readFloat(): float
  readDouble(): double
  readBytes(length: long): readonly byte[]
  readString(length: long): string
  findNextAny(candidates: readonly byte[]): long | null
}