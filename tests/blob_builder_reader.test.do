import { BlobBuilder, BlobReader, Endian } from "../index"

function assertBytes(actual: readonly byte[], expected: readonly byte[]): void {
  assert(actual.length == expected.length, "expected blob lengths to match")

  for index of 0..<actual.length {
    assert(actual[index] == expected[index], "expected blob bytes to match")
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