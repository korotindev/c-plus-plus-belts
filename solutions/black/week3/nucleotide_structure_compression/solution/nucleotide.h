#include <stdint.h>

#include <cstddef>
#include <unordered_map>

struct Nucleotide {
  char Symbol;
  size_t Position;
  int ChromosomeNum;
  int GeneNum;
  bool IsMarked;
  char ServiceInfo;
};

struct CompactNucleotide {
  uint32_t Position;           // min/max : 0 / 3,3 * 10^9
  uint32_t GeneNum : 15;       // 26K max
  uint32_t Symbol : 2;         // 4 different values only
  uint32_t ChromosomeNum : 6;  // 46 different values only
  uint32_t IsMarked : 1;       // packed bool LOL
  uint32_t ServiceInfo : 8;    // ???
};

bool operator==(const Nucleotide& lhs, const Nucleotide& rhs) {
  return (lhs.Symbol == rhs.Symbol) && (lhs.Position == rhs.Position) && (lhs.ChromosomeNum == rhs.ChromosomeNum) &&
         (lhs.GeneNum == rhs.GeneNum) && (lhs.IsMarked == rhs.IsMarked) && (lhs.ServiceInfo == rhs.ServiceInfo);
}

CompactNucleotide Compress(const Nucleotide& n) {
  std::unordered_map<char, uint32_t> sym_mapper{{'A', 0}, {'T', 1}, {'G', 2}, {'C', 3}};

  return {.Position = static_cast<uint32_t>(n.Position),
          .GeneNum = static_cast<uint32_t>(n.GeneNum),
          .Symbol = sym_mapper.at(n.Symbol),
          .ChromosomeNum = static_cast<uint32_t>(n.ChromosomeNum),
          .IsMarked = static_cast<uint32_t>(n.IsMarked),
          .ServiceInfo = static_cast<uint32_t>(n.ServiceInfo)};
}

Nucleotide Decompress(const CompactNucleotide& cn) {
  std::unordered_map<unsigned char, char> sym_mapper{{0, 'A'}, {1, 'T'}, {2, 'G'}, {3, 'C'}};

  return {.Symbol = sym_mapper.at(cn.Symbol),
          .Position = static_cast<size_t>(cn.Position),
          .ChromosomeNum = static_cast<int>(cn.ChromosomeNum),
          .GeneNum = static_cast<int>(cn.GeneNum),
          .IsMarked = static_cast<bool>(cn.IsMarked),
          .ServiceInfo = static_cast<char>(cn.ServiceInfo)};
}