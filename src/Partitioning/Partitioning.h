#pragma once

#include <array>

class Partitioning {
   public:
    Partitioning() = default;

    //! Optional compatibility constructor
    Partitioning(int ownRankNo, int nRanks, std::array<int, 2> nCells);

    //! compute partitioning, set internal variables
    void initialize(std::array<int, 2> nCellsGlobal);

    //! get the local number of cells in the own subdomain
    std::array<int, 2> nCellsLocal() const;

    //! get the global number of cells in the whole computational domain
    std::array<int, 2> nCellsGlobal() const;

    //! get the own MPI rank no
    int ownRankNo() const;

    //! number of MPI ranks
    int nRanks() const;

    bool ownPartitionContainsBottomBoundary() const;
    bool ownPartitionContainsTopBoundary() const;
    bool ownPartitionContainsLeftBoundary() const;
    bool ownPartitionContainsRightBoundary() const;

    int leftNeighbourRankNo() const;
    int rightNeighbourRankNo() const;
    int topNeighbourRankNo() const;
    int bottomNeighbourRankNo() const;

    //! (i_local,j_local) + nodeOffset = (i_global,j_global)
    std::array<int, 2> nodeOffset() const;

    // Additional compatibility methods used by this project
    std::array<int, 2> ownRankCoordinate() const;
    std::array<int, 4> ownPartitionNeighbours() const;
    int ownRightNeighbour() const;
    int ownLeftNeighbour() const;
    int ownTopNeighbour() const;
    int ownBottomNeighbour() const;

    double DURATION_COMPUTATION = 0;
    double DURATION_COMMUNICATION_REDUCE = 0;
    double DURATION_COMMUNICATION_EXCHANGE = 0;
    double DURATION_RESIDUAL_NORM = 0;
    double DURATION_TIME_STEP = 0;
    double DURATION_OUTPUT = 0;
    double DURATION_TOTAL = 0;

   private:
    void setupPartitioning();
    void factorizeSubdomains();

    std::array<int, 2> nodeOffset_{{0, 0}};
    std::array<int, 2> nCellsGlobal_{{0, 0}};
    std::array<int, 2> nCellsLocal_{{0, 0}};
    std::array<int, 2> ownRankCoordinate_{{0, 0}};
    std::array<int, 2> nSubdomains_{{1, 1}};
    std::array<int, 4> partitionNeighbours_{{-1, -1, -1, -1}};
    int ownRankNo_ = 0;
    int nRanks_ = 1;
};
