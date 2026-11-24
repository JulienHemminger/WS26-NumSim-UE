#include "Partitioning.h"

#include <mpi.h>

#include <stdexcept>

Partitioning::Partitioning(int ownRankNo, int nRanks, std::array<int, 2> nCells)
    : ownRankNo_(ownRankNo), nRanks_(nRanks) {
    nCellsGlobal_ = nCells;
    setupPartitioning();
}

void Partitioning::initialize(std::array<int, 2> nCellsGlobal) {
    nCellsGlobal_ = nCellsGlobal;
    MPI_Comm_rank(MPI_COMM_WORLD, &ownRankNo_);
    MPI_Comm_size(MPI_COMM_WORLD, &nRanks_);
    setupPartitioning();
}

void Partitioning::setupPartitioning() {
    factorizeSubdomains();

    const int nSubdomainsX = nSubdomains_[0];
    const int nSubdomainsY = nSubdomains_[1];
    const int processColumn = ownRankNo_ % nSubdomainsX;
    const int processRow = ownRankNo_ / nSubdomainsX;
    ownRankCoordinate_ = {processColumn, processRow};

    partitionNeighbours_[0] = processRow == 0 ? MPI_PROC_NULL : ownRankNo_ - nSubdomainsX;
    partitionNeighbours_[1] = processColumn == 0 ? MPI_PROC_NULL : ownRankNo_ - 1;
    partitionNeighbours_[2] =
        processRow == nSubdomainsY - 1 ? MPI_PROC_NULL : ownRankNo_ + nSubdomainsX;
    partitionNeighbours_[3] = processColumn == nSubdomainsX - 1 ? MPI_PROC_NULL : ownRankNo_ + 1;

    if (nCellsGlobal_[0] % nSubdomainsX != 0 || nCellsGlobal_[1] % nSubdomainsY != 0) {
        throw std::runtime_error(
            "Global number of cells must be divisible by the number of subdomains");
    }

    nCellsLocal_ = {nCellsGlobal_[0] / nSubdomainsX, nCellsGlobal_[1] / nSubdomainsY};
    nodeOffset_ = {nCellsLocal_[0] * processColumn, nCellsLocal_[1] * processRow};
}

void Partitioning::factorizeSubdomains() {
    nSubdomains_ = {1, nRanks_};
    int lowestCost = nCellsGlobal_[0] * (nRanks_ - 1);

    for (int n = 1; n <= nRanks_; ++n) {
        if (nRanks_ % n != 0) {
            continue;
        }

        const int m = nRanks_ / n;
        const int cost = nCellsGlobal_[0] * (m - 1) + nCellsGlobal_[1] * (n - 1);
        if (cost < lowestCost) {
            lowestCost = cost;
            nSubdomains_ = {n, m};
        }
    }
}

std::array<int, 2> Partitioning::nodeOffset() const { return nodeOffset_; }

std::array<int, 2> Partitioning::nCellsGlobal() const { return nCellsGlobal_; }

std::array<int, 2> Partitioning::nCellsLocal() const { return nCellsLocal_; }

int Partitioning::ownRankNo() const { return ownRankNo_; }

int Partitioning::nRanks() const { return nRanks_; }

std::array<int, 2> Partitioning::ownRankCoordinate() const { return ownRankCoordinate_; }

bool Partitioning::ownPartitionContainsBottomBoundary() const {
    return bottomNeighbourRankNo() == MPI_PROC_NULL;
}

bool Partitioning::ownPartitionContainsLeftBoundary() const {
    return leftNeighbourRankNo() == MPI_PROC_NULL;
}

bool Partitioning::ownPartitionContainsTopBoundary() const {
    return topNeighbourRankNo() == MPI_PROC_NULL;
}

bool Partitioning::ownPartitionContainsRightBoundary() const {
    return rightNeighbourRankNo() == MPI_PROC_NULL;
}

int Partitioning::leftNeighbourRankNo() const { return partitionNeighbours_[1]; }

int Partitioning::rightNeighbourRankNo() const { return partitionNeighbours_[3]; }

int Partitioning::topNeighbourRankNo() const { return partitionNeighbours_[2]; }

int Partitioning::bottomNeighbourRankNo() const { return partitionNeighbours_[0]; }

std::array<int, 4> Partitioning::ownPartitionNeighbours() const { return partitionNeighbours_; }

int Partitioning::ownRightNeighbour() const { return rightNeighbourRankNo(); }

int Partitioning::ownLeftNeighbour() const { return leftNeighbourRankNo(); }

int Partitioning::ownTopNeighbour() const { return topNeighbourRankNo(); }

int Partitioning::ownBottomNeighbour() const { return bottomNeighbourRankNo(); }
