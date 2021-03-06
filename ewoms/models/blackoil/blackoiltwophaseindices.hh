// -*- mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
// vi: set et ts=4 sw=4 sts=4:
/*
  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.

  Consult the COPYING file in the top-level source directory of this
  module for the precise wording of the license and the list of
  copyright holders.
*/
/*!
 * \file
 *
 * \copydoc Ewoms::BlackOilTwoPhaseIndices
 */
#ifndef EWOMS_BLACK_OIL_TWO_PHASE_INDICES_HH
#define EWOMS_BLACK_OIL_TWO_PHASE_INDICES_HH

#include <cassert>

namespace Ewoms {

/*!
 * \ingroup BlackOilModel
 *
 * \brief The primary variable and equation indices for the black-oil model.
 */
template <bool enableSolventV, bool enablePolymerV, bool enableEnergyV, unsigned PVOffset, unsigned disabledCanonicalCompIdx>
struct BlackOilTwoPhaseIndices
{
    //! Is phase enabled or not
    static const bool oilEnabled = (disabledCanonicalCompIdx != 0);
    static const bool waterEnabled = (disabledCanonicalCompIdx != 1);
    static const bool gasEnabled = (disabledCanonicalCompIdx != 2);

    //! Are solvents involved?
    static const bool enableSolvent = enableSolventV;

    //! Are polymers involved?
    static const bool enablePolymer = enablePolymerV;

    //! Shall energy be conserved?
    static const bool enableEnergy = enableEnergyV;

private:
    //! Number of solvent components to be considered
    static const int numSolvents_ = enableSolvent ? 1 : 0;

    //! Number of polymer components to be considered
    static const int numPolymers_ = enablePolymer ? 1 : 0;

    //! Number of energy equations to be considered
    static const int numEnergy_ = enableEnergy ? 1 : 0;

public:
    //! The number of fluid phases
    static const int numPhases = 2;

    //! The number of equations
    static const int numEq = numPhases + numSolvents_ + numPolymers_ + numEnergy_;

    //////////////////////////////
    // Primary variable indices
    //////////////////////////////

    //! The index of the water saturation. For two-phase oil gas models this is disabled.
    static const int waterSaturationIdx  = waterEnabled ? PVOffset + 0 : -10000;

    //! Index of the oil pressure in a vector of primary variables
    static const int pressureSwitchIdx  = waterEnabled ? PVOffset + 1 : PVOffset + 0;

    /*!
     * \brief Index of the switching variable which determines the composition of the
     *        hydrocarbon phases.
     *
     * \note For two-phase water oil models this is disabled.
     */
    static const int compositionSwitchIdx = gasEnabled ? PVOffset + 1 : -10000;

    //! Index of the primary variable for the first solvent
    static const int solventSaturationIdx =
        enableSolvent ? PVOffset + (numPhases-1) + numSolvents_ : -1000;

    //! Index of the primary variable for the first polymer
    static const int polymerConcentrationIdx =
        enablePolymer ? PVOffset + (numPhases-1) + numSolvents_ + numPolymers_ : -1000;

    //! Index of the primary variable for temperature
    static const int temperatureIdx  =
        enableEnergy ? PVOffset + (numPhases-1) + numSolvents_ + numPolymers_ + numEnergy_ : - 1000;

    //////////////////////
    // Equation indices
    //////////////////////

    //! \brief returns the index of "active" component
    static unsigned canonicalToActiveComponentIndex(unsigned compIdx)
    {
        // assumes canonical oil = 0, water = 1, gas = 2;
        if(!gasEnabled) {
            assert(compIdx != 2);
            // oil = 0, water = 1
            return compIdx;
        } else if (!waterEnabled) {
            assert(compIdx != 1);
            // oil = 0, gas = 1
            return compIdx / 2;
        } else {
            assert(!oilEnabled);
            assert(compIdx != 0);
        }
        // water = 0, gas = 1;
        return compIdx-1;
    }

    static unsigned activeToCanonicalComponentIndex(unsigned compIdx)
    {
        // assumes canonical oil = 0, water = 1, gas = 2;
        assert(compIdx < 2);
        if(!gasEnabled) {
            // oil = 0, water = 1
            return compIdx;
        } else if (!waterEnabled) {
            // oil = 0, gas = 1
            return compIdx * 2;
        } else {
            assert(!oilEnabled);
        }
        // water = 0, gas = 1;
        return compIdx+1;
    }

    //! Index of the continuity equation of the first phase
    static const int conti0EqIdx = PVOffset + 0;
    // one continuity equation follows

    //! Index of the continuity equation for the first solvent component
    static const int contiSolventEqIdx =
        enableSolvent ? PVOffset + (numPhases-1) + numSolvents_ : -1000;

    //! Index of the continuity equation for the first polymer component
    static const int contiPolymerEqIdx =
        enablePolymer > 0 ? PVOffset + (numPhases-1) + numSolvents_ + numPolymers_ : -1000;

    //! Index of the continuity equation for energy
    static const int contiEnergyEqIdx =
        enableEnergy ? PVOffset + (numPhases-1) + numSolvents_ + numPolymers_ + numEnergy_ : -1000;
};

} // namespace Ewoms

#endif
