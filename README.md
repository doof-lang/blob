# std/blob

Binary data serialization and deserialization. Provides `BlobBuilder` for writing typed values into a byte buffer and `BlobReader` for reading them back, with configurable byte-order (endianness).

## Usage

```doof
import { BlobBuilder, BlobReader, Endian, TextEncoding } from "std/blob"

// Write
builder := BlobBuilder { endianness: .BigEndian }
builder.writeInt(16909060)
builder.writeByte(5)
try! builder.writeText("café", .Windows1252)
builder.writeTextLossy("price: €", .Ascii)
data := builder.build()

// Read
reader := BlobReader(data)
value := reader.readInt()
flag := reader.readByte()
text := try! reader.readText(4L, .Windows1252)
lossyText := reader.readTextLossy(8L, .Ascii)
```

## Exports

### `Endian`

Controls the byte order used by `BlobBuilder` and `BlobReader`.

| Member | Value | Description |
|--------|-------|-------------|
| `BigEndian` | `0` | Most-significant byte first |
| `LittleEndian` | `1` | Least-significant byte first (default) |

### `TextEncoding`

Controls how `writeText` encodes strings and how `readText` decodes bytes.

| Member | Description |
|--------|-------------|
| `Utf8` | UTF-8 |
| `Utf16LE` | UTF-16 little-endian |
| `Utf16BE` | UTF-16 big-endian |
| `Latin1` | ISO-8859-1 |
| `Windows1252` | Windows code page 1252 |
| `CP437` | IBM PC code page 437 |
| `Ascii` | 7-bit ASCII |

### `EncodingError`

Returned by text encoding operations.

| Member | Description |
|--------|-------------|
| `InvalidData` | Input bytes or string data are malformed for the requested encoding |
| `UnrepresentableCharacter` | A string contains a character that cannot be represented by the requested encoding |
| `OutputTooLarge` | The encoded text is too large to report as an `int` byte count |

---

### `BlobBuilder`

Writes typed values into a growable byte buffer. Call `build()` to obtain the final `readonly byte[]`. Building resets the position and length back to zero so the builder can be reused.

#### Construction

```doof
BlobBuilder()                                     // default: LittleEndian, empty
BlobBuilder { endianness: .BigEndian }
BlobBuilder { size: 256L, endianness: .LittleEndian }
```

#### Methods

| Method | Return | Description |
|--------|--------|-------------|
| `getPosition(): long` | `long` | Current write cursor position |
| `setPosition(position: long): void` | — | Move the write cursor; zero-fills gaps |
| `length(): long` | `long` | Current byte length of the buffer |
| `writeByte(value: byte): void` | — | Write a single byte |
| `writeBool(value: bool): void` | — | Write a boolean as one byte |
| `writeInt(value: int): void` | — | Write a 32-bit signed integer |
| `writeLong(value: long): void` | — | Write a 64-bit signed integer |
| `writeFloat(value: float): void` | — | Write a 32-bit float |
| `writeDouble(value: double): void` | — | Write a 64-bit float |
| `writeBytes(value: readonly byte[]): void` | — | Append a raw byte array |
| `writeString(value: string): void` | — | Append raw UTF-8 bytes (no length prefix) |
| `writeText(value: string, encoding: TextEncoding = .Utf8): Result<int, EncodingError>` | `Result<int, EncodingError>` | Encode text, append the bytes, and return the byte count |
| `writeTextLossy(value: string, encoding: TextEncoding = .Utf8): int` | `int` | Encode text, replacing unrepresentable characters with `?`, append the bytes, and return the byte count |
| `build(): readonly byte[]` | `readonly byte[]` | Finalise and return the buffer; resets builder |

---

### `BlobReader`

Reads typed values sequentially from a `readonly byte[]`. The read cursor advances automatically after each call.

#### Construction

```doof
BlobReader(data)                                  // default: LittleEndian, offset 0
BlobReader { data: bytes, endianness: .BigEndian }
```

#### Methods

| Method | Return | Description |
|--------|--------|-------------|
| `getPosition(): long` | `long` | Current read cursor position |
| `setPosition(position: long): void` | — | Seek to an absolute byte offset |
| `length(): long` | `long` | Total byte length of the data |
| `remaining(): long` | `long` | Unread bytes remaining |
| `readByte(): byte` | `byte` | Read one byte |
| `readBool(): bool` | `bool` | Read one byte as boolean |
| `readInt(): int` | `int` | Read a 32-bit signed integer |
| `readLong(): long` | `long` | Read a 64-bit signed integer |
| `readFloat(): float` | `float` | Read a 32-bit float |
| `readDouble(): double` | `double` | Read a 64-bit float |
| `readBytes(length: long): readonly byte[]` | `readonly byte[]` | Read `length` raw bytes |
| `readString(length: long): string` | `string` | Read `length` bytes as a UTF-8 string |
| `readText(length: long, encoding: TextEncoding = .Utf8): Result<string, EncodingError>` | `Result<string, EncodingError>` | Decode `length` bytes as text |
| `readTextLossy(length: long, encoding: TextEncoding = .Utf8): string` | `string` | Decode `length` bytes as text, replacing malformed input with `�` |
| `findNextAny(candidates: readonly byte[]): long \| null` | `long \| null` | Return the offset of the next byte matching any candidate, or `null` |
