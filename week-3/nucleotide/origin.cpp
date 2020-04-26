#include <cstddef>
#include <cstdint>


struct Nucleotide {
  char Symbol;
  size_t Position;
  int ChromosomeNum;
  int GeneNum;
  bool IsMarked;
  char ServiceInfo;
};


struct CompactNucleotide {
  uint32_t Position;
  uint16_t GeneNum:15;
  uint16_t IsMarked:1;
  uint8_t Symbol:2;
  uint8_t ChromosomeNum:6;
  char ServiceInfo;
};

static_assert(sizeof(CompactNucleotide) <= 8, "Your CompactNucleotide is not compact enough");
static_assert(alignof(CompactNucleotide) == 4, "Don't use '#pragma pack'!");


bool operator == (const Nucleotide& lhs, const Nucleotide& rhs) {
  return (lhs.Symbol == rhs.Symbol)
      && (lhs.Position == rhs.Position)
      && (lhs.ChromosomeNum == rhs.ChromosomeNum)
      && (lhs.GeneNum == rhs.GeneNum)
      && (lhs.IsMarked == rhs.IsMarked)
      && (lhs.ServiceInfo == rhs.ServiceInfo);
}


CompactNucleotide Compress(const Nucleotide& n) {
  CompactNucleotide result;
  result.Position = n.Position;
  result.GeneNum = n.GeneNum;
  result.IsMarked = n.IsMarked;
  result.ChromosomeNum = n.ChromosomeNum;
  result.ServiceInfo = n.ServiceInfo;

  switch (n.Symbol) {
    case 'A':
      result.Symbol = 0; break;
    case 'T':
      result.Symbol = 1; break;
    case 'G':
      result.Symbol = 2; break;
    case 'C':
      result.Symbol = 3; break;
    default:
      throw "Impossible value of Nucleotide symbol";
  }

  return result;
};


Nucleotide Decompress(const CompactNucleotide& cn) {
  Nucleotide result;
  result.Position = cn.Position;
  result.GeneNum = cn.GeneNum;
  result.IsMarked = cn.IsMarked;
  result.ChromosomeNum = cn.ChromosomeNum;
  result.ServiceInfo = cn.ServiceInfo;

  switch (cn.Symbol) {
    case 0:
      result.Symbol = 'A'; break;
    case 1:
      result.Symbol = 'T'; break;
    case 2:
      result.Symbol = 'G'; break;
    case 3:
      result.Symbol = 'C'; break;
    default:
      throw "Impossible value of Nucleotide symbol";
  }

  return result;
}
