# std/blob Guide

`std/blob` is the low-level binary serialization package. Use `BlobBuilder` to
write typed values into a growable byte buffer, and `BlobReader` to read typed
values back from a `readonly byte[]`.

The package is intentionally cursor-based. Each read or write advances the
current position unless the method explicitly says otherwise.

## Quick Start

```doof
import { BlobBuilder, BlobReader, Endian, TextEncoding } from "std/blob"

builder := BlobBuilder { endianness: .BigEndian }
builder.writeInt(16909060)
try! builder.writeText("cafe", .Utf8)
data := builder.build()

reader := BlobReader { data, endianness: .BigEndian }
number := reader.readInt()
text := try! reader.readText(reader.remaining(), .Utf8)
```

## Byte Order

`Endian` controls multi-byte integer and floating-point values:

- `BigEndian` writes the most-significant byte first.
- `LittleEndian` writes the least-significant byte first and is the default.

Use the same endianness when reading and writing a binary format. Single bytes
and raw byte arrays are not affected by endianness.

## Cursor And Alignment

`BlobBuilder.setPosition(position)` moves the write cursor. Moving past the
current end zero-fills the gap. `align(width)` advances to the next multiple of
`width`, also zero-filling on writes.

`BlobReader.setPosition(position)` seeks to an absolute offset. `skip(length)`
advances relative to the current offset, and `peekByte()` reads without moving
the cursor.

`BlobBuilder.build()` returns the current bytes and resets the builder so it can
be reused.

## Text Encodings

`writeString` and `readString` use raw UTF-8 bytes and do not add or consume a
length prefix.

`writeText` and `readText` support explicit encodings:

- `Utf8`
- `Utf16LE`
- `Utf16BE`
- `Latin1`
- `Windows1252`
- `CP437`
- `Ascii`

Strict text conversion returns `Result<..., EncodingError>`. Lossy conversion
replaces unrepresentable output characters with `?` and malformed input with the
replacement character.

## API

### Enums

```doof
export enum Endian
export enum TextEncoding
export enum EncodingError
```

Defined in [types.do](../types.do).

### `BlobBuilder`

```doof
export import class BlobBuilder
```

Constructor:

- `BlobBuilder(size: long = 0L, endianness: Endian = .LittleEndian)`

Cursor and size:

- `getPosition(): long`
- `setPosition(position: long): void`
- `length(): long`
- `align(width: long): void`
- `writeZeroes(length: long): void`

Writes:

- `writeByte(value: byte): void`
- `writeSignedByte(value: int): void`
- `writeBool(value: bool): void`
- `writeShort(value: int): void`
- `writeUnsignedShort(value: int): void`
- `writeInt(value: int): void`
- `writeUnsignedInt(value: long): void`
- `writeLong(value: long): void`
- `writeFloat(value: float): void`
- `writeDouble(value: double): void`
- `writeBytes(value: readonly byte[]): void`
- `writeString(value: string): void`
- `writeText(value: string, encoding: TextEncoding = .Utf8): Result<int, EncodingError>`
- `writeTextLossy(value: string, encoding: TextEncoding = .Utf8): int`
- `build(): readonly byte[]`

Defined in [index.do](../index.do).

### `BlobReader`

```doof
export import class BlobReader
```

Constructor:

- `BlobReader(data: readonly byte[], endianness: Endian = .LittleEndian)`

Cursor and size:

- `getPosition(): long`
- `setPosition(position: long): void`
- `length(): long`
- `remaining(): long`
- `skip(length: long): void`
- `align(width: long): void`

Reads:

- `peekByte(): byte`
- `readByte(): byte`
- `readSignedByte(): int`
- `readBool(): bool`
- `readShort(): int`
- `readUnsignedShort(): int`
- `readInt(): int`
- `readUnsignedInt(): long`
- `readLong(): long`
- `readFloat(): float`
- `readDouble(): double`
- `readBytes(length: long): readonly byte[]`
- `readString(length: long): string`
- `readText(length: long, encoding: TextEncoding = .Utf8): Result<string, EncodingError>`
- `readTextLossy(length: long, encoding: TextEncoding = .Utf8): string`
- `findNextAny(candidates: readonly byte[]): long | null`

Defined in [index.do](../index.do).
