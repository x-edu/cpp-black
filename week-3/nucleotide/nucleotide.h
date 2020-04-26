#include <cstddef>

struct Nucleotide {
  char Symbol;
  size_t Position;
  int ChromosomeNum;
  int GeneNum;
  bool IsMarked;
  char ServiceInfo;
};

struct CompactNucleotide {
  unsigned long long Symbol : 2;
  unsigned long long ChromosomeNum : 6;
  unsigned long long GeneNum : 15;
  unsigned long long IsMarked : 1;
  unsigned long long ServiceInfo : 8;
  unsigned long long Position : 32;
};

bool operator==(const Nucleotide& lhs, const Nucleotide& rhs) {
  return (lhs.Symbol == rhs.Symbol) && (lhs.Position == rhs.Position) &&
         (lhs.ChromosomeNum == rhs.ChromosomeNum) &&
         (lhs.GeneNum == rhs.GeneNum) && (lhs.IsMarked == rhs.IsMarked) &&
         (lhs.ServiceInfo == rhs.ServiceInfo);
}

unsigned long long CompressSymbol(char symbol) {
  switch (symbol) {
    case 'A':
      return 0;
    case 'T':
      return 1;
    case 'G':
      return 2;
    case 'C':
      return 3;
    default:
      return 0;
  }
}

char DecompressSymbol(unsigned long long symbol) {
  switch (symbol) {
    case 0:
      return 'A';
    case 1:
      return 'T';
    case 2:
      return 'G';
    case 3:
      return 'C';
    default:
      return 'A';
  }
}

CompactNucleotide Compress(const Nucleotide& n) {
  return {
      .Symbol = CompressSymbol(n.Symbol),
      .ChromosomeNum = static_cast<unsigned long long>(n.ChromosomeNum - 1),
      .GeneNum = static_cast<unsigned long long>(n.GeneNum),
      .IsMarked = n.IsMarked,
      .ServiceInfo = static_cast<unsigned long long>(n.ServiceInfo),
      .Position = (unsigned long long)n.Position,
  };
}

Nucleotide Decompress(const CompactNucleotide& cn) {
  return {
      .Symbol = DecompressSymbol(cn.Symbol),
      .Position = static_cast<size_t>(cn.Position),
      .ChromosomeNum = cn.ChromosomeNum + 1,
      .GeneNum = cn.GeneNum,
      .IsMarked = static_cast<bool>(cn.IsMarked),
      .ServiceInfo = static_cast<char>(cn.ServiceInfo),
  };
}
