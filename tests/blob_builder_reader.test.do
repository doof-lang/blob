import { BlobBuilder, BlobReader, EncodingError, Endian, TextEncoding } from "../index"

function assertBytes(actual: readonly byte[], expected: readonly byte[]): void {
  assert(actual.length == expected.length, "expected blob lengths to match")

  for index of 0..<actual.length {
    assert(actual[index] == expected[index], "expected blob bytes to match")
  }
}

function isFailure<T, E>(result: Result<T, E>): bool {
  return case result {
    _: Success -> false,
    _: Failure -> true,
  }
}

function assertEncodingError<T>(result: Result<T, EncodingError>, expected: EncodingError): void {
  return case result {
    _: Success -> assert(false, "expected encoding operation to fail"),
    failure: Failure -> assert(failure.error == expected, "expected encoding error to match"),
  }
}

export function testAll() {
  builder := BlobBuilder{endianness: .BigEndian}
  builder.writeInt(16909060)
  builder.writeByte(5)

  built := builder.build()
  expectedBuilt: readonly byte[] := [1, 2, 3, 4, 5]
  assertBytes(built, expectedBuilt)
  assert(builder.length() == 0L, "expected build() to reset the builder length")
  assert(builder.getPosition() == 0L, "expected build() to reset the builder position")

  gapBuilder := BlobBuilder()
  gapBuilder.setPosition(2L)
  gapBuilder.writeByte(7)
  gapBytes := gapBuilder.build()
  expectedGap: readonly byte[] := [0, 0, 7]
  assertBytes(gapBytes, expectedGap)

  littleBuilder := BlobBuilder { endianness: .LittleEndian }
  payloadPrefix: readonly byte[] := [9, 8, 7]
  littleBuilder.writeBytes(payloadPrefix)
  littleBuilder.writeBool(true)
  littleBuilder.writeInt(16909060)
  littleBuilder.writeLong(72623859790382856L)
  littleBuilder.writeDouble(12.5)
  littleBuilder.writeString("hé")

  littleBytes := littleBuilder.build()
  expectedLittle: readonly byte[] := [
    9, 8, 7,
    1,
    4, 3, 2, 1,
    8, 7, 6, 5, 4, 3, 2, 1,
    0, 0, 0, 0, 0, 0, 41, 64,
    104, 195, 169,
  ]
  assertBytes(littleBytes, expectedLittle)

  reader := BlobReader { data: littleBytes, endianness: .LittleEndian }
  readPrefix := reader.readBytes(3L)
  assertBytes(readPrefix, payloadPrefix)
  assert(reader.readBool(), "expected bool payload to round-trip")
  assert(reader.readInt() == 16909060, "expected int payload to round-trip")
  assert(reader.readLong() == 72623859790382856L, "expected long payload to round-trip")
  assert(reader.readDouble() == 12.5, "expected double payload to round-trip")
  assert(reader.readString(3L) == "hé", "expected UTF-8 string payload to round-trip")
  assert(reader.remaining() == 0L, "expected all bytes to be consumed")

  searchCandidates: readonly byte[] := [30, 50]
  searchData: readonly byte[] := [10, 20, 30, 40, 50]
  searchReader := BlobReader(searchData)
  firstFound := searchReader.findNextAny(searchCandidates)
  assert(firstFound != null && firstFound == 2L, "expected search to find the first matching byte")
  assert(searchReader.getPosition() == 0L, "expected search to leave the reader position unchanged")

  searchReader.setPosition(3L)
  laterFound := searchReader.findNextAny(searchCandidates)
  assert(laterFound != null && laterFound == 4L, "expected search to start from the current reader position")

  searchReader.setPosition(5L)
  assert(searchReader.findNextAny(searchCandidates) == null, "expected search to return null when no match remains")
}

export function testTextEncodingsRoundTrip(): void {
  builder := BlobBuilder()
  assert((try! builder.writeText("hé", .Utf8)) == 3, "expected UTF-8 byte count")
  assert((try! builder.writeText("AΩ", .Utf16LE)) == 4, "expected UTF-16LE byte count")
  assert((try! builder.writeText("€", .Utf16BE)) == 2, "expected UTF-16BE byte count")
  assert((try! builder.writeText("é", .Latin1)) == 1, "expected Latin1 byte count")
  assert((try! builder.writeText("€", .Windows1252)) == 1, "expected Windows-1252 byte count")
  assert((try! builder.writeText("Ç", .CP437)) == 1, "expected CP437 byte count")
  assert((try! builder.writeText("ASCII", .Ascii)) == 5, "expected ASCII byte count")

  bytes := builder.build()
  expected: readonly byte[] := [
    104, 195, 169,
    65, 0, 169, 3,
    32, 172,
    233,
    128,
    128,
    65, 83, 67, 73, 73,
  ]
  assertBytes(bytes, expected)

  reader := BlobReader(bytes)
  assert((try! reader.readText(3L, .Utf8)) == "hé", "expected UTF-8 text to round-trip")
  assert((try! reader.readText(4L, .Utf16LE)) == "AΩ", "expected UTF-16LE text to round-trip")
  assert((try! reader.readText(2L, .Utf16BE)) == "€", "expected UTF-16BE text to round-trip")
  assert((try! reader.readText(1L, .Latin1)) == "é", "expected Latin1 text to round-trip")
  assert((try! reader.readText(1L, .Windows1252)) == "€", "expected Windows-1252 text to round-trip")
  assert((try! reader.readText(1L, .CP437)) == "Ç", "expected CP437 text to round-trip")
  assert((try! reader.readText(5L, .Ascii)) == "ASCII", "expected ASCII text to round-trip")
  assert(reader.remaining() == 0L, "expected text reads to consume all bytes")
}

export function testTextEncodingFailures(): void {
  builder := BlobBuilder()
  assertEncodingError(builder.writeText("é", .Ascii), .UnrepresentableCharacter)
  assert(builder.length() == 0L, "expected failed writeText to leave builder unchanged")

  asciiReader := BlobReader([128])
  assertEncodingError(asciiReader.readText(1L, .Ascii), .InvalidData)
  assert(asciiReader.getPosition() == 0L, "expected failed readText to leave reader position unchanged")

  utf8Reader := BlobReader([195, 40])
  assertEncodingError(utf8Reader.readText(2L, .Utf8), .InvalidData)

  utf16Reader := BlobReader([0, 216])
  assertEncodingError(utf16Reader.readText(2L, .Utf16LE), .InvalidData)

  assert(isFailure(builder.writeText("Ω", .Latin1)), "expected Latin1 to reject unrepresentable text")
}

export function testTextEncodingLossyWritesReplacementQuestionMarks(): void {
  builder := BlobBuilder()
  assert(builder.writeTextLossy("héΩ", .Ascii) == 3, "expected ASCII lossy byte count")
  assert(builder.writeTextLossy("AΩ", .Latin1) == 2, "expected Latin1 lossy byte count")
  assert(builder.writeTextLossy("Ω", .Windows1252) == 1, "expected Windows-1252 lossy byte count")

  expected: readonly byte[] := [
    104, 63, 63,
    65, 63,
    63,
  ]
  assertBytes(builder.build(), expected)
}

export function testTextEncodingLossyReadsReplacementCharacters(): void {
  asciiReader := BlobReader([65, 128, 66])
  assert(asciiReader.readTextLossy(3L, .Ascii) == "A�B", "expected invalid ASCII bytes to decode lossily")
  assert(asciiReader.getPosition() == 3L, "expected lossy ASCII read to advance")

  utf8Reader := BlobReader([195, 40])
  assert(utf8Reader.readTextLossy(2L, .Utf8) == "�(", "expected invalid UTF-8 bytes to decode lossily")
  assert(utf8Reader.getPosition() == 2L, "expected lossy UTF-8 read to advance")

  utf16Reader := BlobReader([0, 216, 65])
  assert(utf16Reader.readTextLossy(3L, .Utf16LE) == "��", "expected invalid UTF-16 bytes to decode lossily")
  assert(utf16Reader.getPosition() == 3L, "expected lossy UTF-16 read to advance")
}
