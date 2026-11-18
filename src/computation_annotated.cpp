#include "Computation.h"

void Computation::initialize(int argc, char* argv[]) {
    settings_.loadFromFile(argv[1]);
    settings_.printSettings();

    // 2. Calculate mesh widths (h_x, h_y) for the rectangular grid cells.
    // physicalSize[0] / nCells[0] gives dx, physicalSize[1] / nCells[1] gives dy.
    for (int i = 0; i < 2; i++) {
        meshWidth_[i] = settings_.physicalSize[i] / settings_.nCells[i];
    }

    // 3. Select spatial discretization scheme for convective terms (Chapter 3.2.4):
    // - Donor Cell scheme: Adds upwinding / artificial diffusion to stabilize convection at high
    // Reynolds numbers (Re >> 1),
    //   controlled by weighting factor alpha. Only 1st order accurate[cite: 17].
    // - Central Differences: Standard 2nd order accurate discretization ($\mathcal{O}(h^2)$),
    //   but prone to numerical oscillations if the flow is convection-dominated[cite: 10, 17].
    if (settings_.useDonorCell) {
        discretization_ =
            std::make_shared<DonorCell>(settings_.nCells, meshWidth_, settings_.alpha);
    } else {
        discretization_ = std::make_shared<CentralDifferences>(settings_.nCells, meshWidth_);
    }

    // 4. Select iterative solver for the Pressure Poisson Equation (Chapter 4):
    // - SOR (Successive Over-Relaxation): Accelerated Gauss-Seidel using relaxation parameter
    // omega,
    //   significantly reducing iteration counts (Chapter 4.2)[cite: 20, 21].
    // - Gauss-Seidel: Standard point-by-point iterative solver (Chapter 4.1)[cite: 19, 20].
    if (settings_.pressureSolver == "SOR") {
        pressureSolver_ =
            std::make_unique<SOR>(discretization_, settings_.epsilon,
                                  settings_.maximumNumberOfIterations, settings_.omega);
    } else if (settings_.pressureSolver == "GaussSeidel") {
        pressureSolver_ = std::make_unique<GaussSeidel>(discretization_, settings_.epsilon,
                                                        settings_.maximumNumberOfIterations);
    } else {
        std::cout << "Invalid pressure solver name: " << settings_.pressureSolver << std::endl;
        throw;
    }

    dt_ = 0.;

    // 5. Initialize output writers for visualization (Paraview VTK files and text/console logs)
    outputWriterParaview_ = std::make_unique<OutputWriterParaview>(discretization_);
    outputWriterText_ = std::make_unique<OutputWriterText>(discretization_);
}

void Computation::runSimulation() {
    double currentTime = 0;

    std::cout << "Starting Time: " << currentTime << std::endl;

    // Main time-stepping loop following the algorithm for one time step (Chapter 3.2.6)[cite: 18]
    while (currentTime < settings_.endTime) {
        // Step 1: Apply boundary values to velocity fields (staggered grid ghost cells / tangential
        // interpolation)[cite: 11, 18]
        applyBoundaryValues();

        // Step 2: Compute adaptive time step width dt based on CFL and diffusion stability limits
        // (Chapter 3.2.5)[cite: 17, 18]
        computeTimeStepWidth();

        // Ensure simulation does not overshoot the configured final end time
        dt_ = std::min(dt_, settings_.endTime - currentTime);
        currentTime += dt_;

        std::cout << "Current Time: " << currentTime << " with time step " << dt_ << std::endl;

        // Step 3: Compute preliminary velocities F and G (Eq. 3.2) using explicit Euler time
        // stepping[cite: 13, 18] Incorporates diffusion, convection, and external body forces
        // (gravity g)[cite: 2, 13]
        computePreliminaryVelocities();

        // Step 4: Compute the right-hand side (RHS) of the Pressure Poisson Equation
        // (Eq. 3.3)[cite: 14, 18] RHS is proportional to the divergence of the preliminary
        // velocities: $\frac{1}{\Delta t} \nabla \cdot (F, G)$[cite: 14]
        computeRightHandSide();

        // Step 5: Solve the Pressure Poisson Equation iteratively (SOR / Gauss-Seidel) to obtain
        // pressure $p^{(n+1)}$ (Chapter 4)[cite: 18, 19]
        solvePressure();

        // Step 6: Compute final velocities $u^{(n+1)}$ and $v^{(n+1)}$ (Eq. 3.4)[cite: 13, 18]
        // Corrects preliminary velocities with the newly computed pressure gradient: $u^{(n+1)} = F
        // - \Delta t \nabla p$[cite: 13]
        computeVelocities();

        // Step 7: Export results for visualization and logging[cite: 18]
        outputWriterParaview_->writeFile(currentTime);
        outputWriterText_->writeFile(currentTime);
    }

    std::cout << "Simulation completed" << std::endl;
}

void Computation::computeTimeStepWidth() {
    double boundary_diffusion = 0.;

    // Calculate viscous diffusion stability upper bound (Chapter 3.2.5)[cite: 17]
    // Ensures stability for the explicit treatment of the diffusion term: $\frac{2}{Re} \Delta t <
    // \frac{\Delta x^2 \Delta y^2}{\Delta x^2 + \Delta y^2}$
    if (meshWidth_[0] == meshWidth_[1]) {
        boundary_diffusion = (settings_.re * meshWidth_[0] * meshWidth_[1]) / 4.;
    } else {
        double h2x = meshWidth_[0] * meshWidth_[0];
        double h2y = meshWidth_[1] * meshWidth_[1];
        boundary_diffusion = (settings_.re / 2.) * (h2x * h2y) * (1. / (h2x + h2y));
    }

    // Lambda helper to find the maximum absolute velocity component across the staggered grid
    const auto maximumAbsoluteVelocity = [](int iBegin, int iEnd, int jBegin, int jEnd,
                                            const auto& velocityAt) {
        double maximum = 0.;
        for (int j = jBegin; j < jEnd; j++) {
            for (int i = iBegin; i < iEnd; i++) {
                maximum = std::max(maximum, std::abs(velocityAt(i, j)));
            }
        }
        return maximum;
    };

    // Extract maximum absolute velocities $u_{max}$ and $v_{max}$ in the domain
    const double u_max = maximumAbsoluteVelocity(
        discretization_->uIBegin(), discretization_->uIEnd(), discretization_->uJBegin(),
        discretization_->uJEnd(), [&](int i, int j) { return discretization_->u(i, j); });
    const double v_max = maximumAbsoluteVelocity(
        discretization_->vIBegin(), discretization_->vIEnd(), discretization_->vJBegin(),
        discretization_->vJEnd(), [&](int i, int j) { return discretization_->v(i, j); });

    // Calculate convective CFL stability upper bounds (Chapter 3.2.5): $|u_{max}|\Delta t < \Delta
    // x$[cite: 17]
    double boundary_convection_u = meshWidth_[0] / u_max;
    double boundary_convection_v = meshWidth_[1] / v_max;

    // The stable time step must satisfy all physical stability constraints (diffusion and
    // convection)
    double min_dt = std::min({boundary_diffusion, boundary_convection_u, boundary_convection_v});

    std::cout << "--- Time Step Breakdown ---" << std::endl;
    std::cout << "Diffusion upper bound: " << boundary_diffusion << std::endl;
    std::cout << "Convection U upper bound: " << boundary_convection_u << " (u_max = " << u_max
              << ")" << std::endl;
    std::cout << "Convection V upper bound: " << boundary_convection_v << " (v_max = " << v_max
              << ")" << std::endl;

    if (min_dt == boundary_diffusion) {
        std::cout << "Limiting stability condition reached: Diffusion operator" << std::endl;
    } else if (min_dt == boundary_convection_u) {
        std::cout << "Limiting stability condition reached: Convection operator (U direction)"
                  << std::endl;
    } else {
        std::cout << "Limiting stability condition reached: Convection operator (V direction)"
                  << std::endl;
    }

    // Apply safety factor tau and upper limit ceiling on dt
    double tau_dt = min_dt * settings_.tau;
    dt_ = std::min(tau_dt, settings_.maximumDt);

    std::cout << "Smallest stability min_dt: " << min_dt << std::endl;
    std::cout << "After applying safety factor tau (" << settings_.tau << "): " << tau_dt
              << std::endl;
    std::cout << "Maximum allowed dt configured: " << settings_.maximumDt << std::endl;
    std::cout << "Final computed dt_: " << dt_ << std::endl;
    std::cout << "---------------------------" << std::endl;
}

void Computation::applyBoundaryValues() {
    // Apply boundary values to velocity fields on the staggered grid (Chapter 3.1.3)[cite: 11]
    // Tangential velocities at boundaries are interpolated (ghost cell formulas like u_interior =
    // 2*u_boundary - u_interior)[cite: 11]

    const auto setHorizontalBoundary = [&](bool top, const std::array<double, 2>& boundaryValues) {
        const int uBoundary = top ? discretization_->uJEnd() - 1 : discretization_->uJBegin();
        const int uInterior = top ? uBoundary - 1 : uBoundary + 1;
        const int vBoundary = top ? discretization_->vJEnd() - 1 : discretization_->vJBegin();

        // Set horizontal velocity u boundary condition (e.g., sliding lid velocity $u_b$)[cite: 11]
        for (int i = discretization_->uIBegin(); i < discretization_->uIEnd(); i++) {
            discretization_->u(i, uBoundary) =
                2. * boundaryValues[0] - discretization_->u(i, uInterior);
            discretization_->f(i, uBoundary) =
                discretization_->u(i, uBoundary);  // Set F at boundary
        }

        // Set vertical velocity v boundary condition (e.g., no-penetration $v=0$)[cite: 11]
        for (int i = discretization_->vIBegin(); i < discretization_->vIEnd(); i++) {
            discretization_->v(i, vBoundary) = boundaryValues[1];
            discretization_->g(i, vBoundary) =
                discretization_->v(i, vBoundary);  // Set G at boundary
        }
    };

    setHorizontalBoundary(true, settings_.dirichletBcTop);
    setHorizontalBoundary(false, settings_.dirichletBcBottom);

    const auto setVerticalBoundary = [&](bool left, const std::array<double, 2>& boundaryValues) {
        const int uBoundary = left ? discretization_->uIBegin() : discretization_->uIEnd() - 1;
        const int vBoundary = left ? discretization_->vIBegin() : discretization_->vIEnd() - 1;
        const int vInterior = left ? vBoundary + 1 : vBoundary - 1;

        // Set horizontal velocity u boundary condition (no-penetration $u=0$)[cite: 11]
        for (int j = discretization_->uJBegin(); j < discretization_->uJEnd(); j++) {
            discretization_->u(uBoundary, j) = boundaryValues[0];
            discretization_->f(uBoundary, j) = discretization_->u(uBoundary, j);
        }

        // Set vertical velocity v boundary condition (tangential interpolation)[cite: 11]
        for (int j = discretization_->vJBegin(); j < discretization_->vJEnd(); j++) {
            discretization_->v(vBoundary, j) =
                2. * boundaryValues[1] - discretization_->v(vInterior, j);
            discretization_->g(vBoundary, j) = discretization_->v(vBoundary, j);
        }
    };

    setVerticalBoundary(true, settings_.dirichletBcLeft);
    setVerticalBoundary(false, settings_.dirichletBcRight);
}

void Computation::computePreliminaryVelocities() {
    // Computes preliminary velocity components F and G using explicit Euler time stepping on the
    // momentum equations (Eq. 3.2)[cite: 13] $F_{ij}^{(n)} = u_{ij}^{(n)} + \Delta t \left(
    // \frac{1}{Re} (\partial_{xx}u + \partial_{yy}u) - \partial_x(u^2) - \partial_y(uv) + g_x
    // \right)$[cite: 13, 17]

    // 1. Compute preliminary x-velocity F for interior nodes
    for (int j = discretization_->uJBegin() + 1; j < discretization_->uJEnd() - 1; j++) {
        for (int i = discretization_->uIBegin() + 1; i < discretization_->uIEnd() - 1; i++) {
            // Diffusion terms: second derivatives of u (Chapter 3.1.2)[cite: 10]
            double diffusion =
                discretization_->computeD2uDx2(i, j) + discretization_->computeD2uDy2(i, j);
            // Convection terms: x-derivative of $u^2$ and y-derivative of $uv$ (Chapter 3.1.2
            // / 3.2.4)[cite: 10, 11, 17]
            double convection =
                -discretization_->computeDu2Dx(i, j) - discretization_->computeDuvDy(i, j);
            // Sum of all physical forces (viscous diffusion, convection, external body
            // forces/gravity)[cite: 2, 13]
            double sum = ((1. / settings_.re) * diffusion) + convection + settings_.g[0];
            // Explicit Euler advance to get F (Eq. 3.2)[cite: 13]
            discretization_->f(i, j) = discretization_->u(i, j) + dt_ * sum;
        }
    }

    // 2. Compute preliminary y-velocity G for interior nodes (analogous to F)[cite: 13]
    for (int j = discretization_->vJBegin() + 1; j < discretization_->vJEnd() - 1; j++) {
        for (int i = discretization_->vIBegin() + 1; i < discretization_->vIEnd() - 1; i++) {
            double diffusion =
                discretization_->computeD2vDx2(i, j) + discretization_->computeD2vDy2(i, j);
            double convection =
                -discretization_->computeDuvDx(i, j) - discretization_->computeDv2Dy(i, j);
            double sum = ((1. / settings_.re) * diffusion) + convection + settings_.g[1];
            discretization_->g(i, j) = discretization_->v(i, j) + dt_ * sum;
        }
    }
}

void Computation::computeRightHandSide() {
    // Computes the right-hand side (RHS) of the Pressure Poisson Equation (Eq. 3.3)[cite: 14]
    // Derived by substituting the discrete momentum equations into the continuity equation ($\nabla
    // \cdot u^{(n+1)} = 0$)[cite: 14] $RHS_{ij} = \frac{1}{\Delta t} \left( \frac{F_{ij} -
    // F_{i-1,j}}{\delta x} + \frac{G_{ij} - G_{i,j-1}}{\delta y} \right)$[cite: 14, 18]
    for (int j = discretization_->pJBegin() + 1; j < discretization_->pJEnd() - 1; j++) {
        for (int i = discretization_->pIBegin() + 1; i < discretization_->pIEnd() - 1; i++) {
            double difference_f =
                (discretization_->f(i, j) - discretization_->f(i - 1, j)) / discretization_->dx();
            double difference_g =
                (discretization_->g(i, j) - discretization_->g(i, j - 1)) / discretization_->dy();
            discretization_->rhs(i, j) = (1 / dt_) * (difference_f + difference_g);
        }
    }
}

void Computation::solvePressure() {
    // Invokes the iterative pressure solver (SOR or Gauss-Seidel) to solve the discrete Poisson
    // equation $Bp = RHS$ (Chapter 4)[cite: 18, 19]
    pressureSolver_->solve();
}

void Computation::computeVelocities() {
    // Compute final divergence-free velocities $u^{(n+1)}$ and $v^{(n+1)}$ after pressure iteration
    // (Chapter 3.2.1 / 3.2.6)[cite: 13, 18] Corrects preliminary velocities F and G using the newly
    // computed pressure gradient $\nabla p^{(n+1)}$[cite: 13] $u_{ij}^{(n+1)} = F_{ij}^{(n)} -
    // \Delta t \frac{\partial p^{(n+1)}}{\partial x}$[cite: 13, 18]
    for (int j = discretization_->uJBegin() + 1; j < discretization_->uJEnd() - 1; j++) {
        for (int i = discretization_->uIBegin() + 1; i < discretization_->uIEnd() - 1; i++) {
            discretization_->u(i, j) =
                discretization_->f(i, j) - dt_ * discretization_->computeDpDx(i, j);
        }
    }

    // $v_{ij}^{(n+1)} = G_{ij}^{(n)} - \Delta t \frac{\partial p^{(n+1)}}{\partial y}$[cite: 13,
    // 18]
    for (int j = discretization_->vJBegin() + 1; j < discretization_->vJEnd() - 1; j++) {
        for (int i = discretization_->vIBegin() + 1; i < discretization_->vIEnd() - 1; i++) {
            discretization_->v(i, j) =
                discretization_->g(i, j) - dt_ * discretization_->computeDpDy(i, j);
        }
    }
}