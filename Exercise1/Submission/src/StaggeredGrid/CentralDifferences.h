#pragma once

#include "StaggeredGrid/Discretization.h"
#include <array>

class CentralDifferences : public Discretization{
    public:
        using Discretization::Discretization;

        //compute the 1st derivative ∂ u^2 / ∂x
        virtual double computeDu2Dx(int i, int j) const override;

        //compute the 1st derivative ∂ v^2 / ∂x
        virtual double computeDv2Dy(int i, int j) const override;

        //compute the 1st derivative ∂ uv / ∂x
        virtual double computeDuvDx(int i, int j) const override;

        //compute the 1st derivative ∂ uv / ∂y
        virtual double computeDuvDy(int i, int j) const override;
};