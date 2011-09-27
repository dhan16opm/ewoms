// -*- mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
// vi: set et ts=4 sw=4 sts=4:
/*****************************************************************************
 *   Copyright (C) 2009 by Andreas Lauser                                    *
 *   Copyright (C) 2009 by Onur Dogan                                        *
 *   Institute for Modelling Hydraulic and Environmental Systems             *
 *   University of Stuttgart, Germany                                        *
 *   email: <givenname>.<name>@iws.uni-stuttgart.de                          *
 *                                                                           *
 *   This program is free software: you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation, either version 2 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU General Public License for more details.                            *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License       *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.   *
 *****************************************************************************/
/*!
 * \file
 *
 * \brief Element-wise calculation of the Jacobian matrix for problems
 *        using the one-phase box model.
 */
#ifndef DUMUX_1P_LOCAL_RESIDUAL_HH
#define DUMUX_1P_LOCAL_RESIDUAL_HH

#include <dumux/boxmodels/common/boxlocalresidual.hh>

#include "1pvolumevariables.hh"
#include "1pfluxvariables.hh"
#include "1pproperties.hh"

namespace Dumux
{
/*!
 * \ingroup OnePBoxModel
 * \ingroup BoxLocalResidual
 * \brief Element-wise calculation of the Jacobian matrix for problems
 *        using the one-phase box model.
 */
template<class TypeTag>
class OnePLocalResidual : public BoxLocalResidual<TypeTag>
{
    typedef OnePLocalResidual<TypeTag> ThisType;

    typedef typename GET_PROP_TYPE(TypeTag, Scalar) Scalar;
    typedef typename GET_PROP_TYPE(TypeTag, PrimaryVariables) PrimaryVariables;
    typedef typename GET_PROP_TYPE(TypeTag, VolumeVariables) VolumeVariables;
    typedef typename GET_PROP_TYPE(TypeTag, FluxVariables) FluxVariables;
    typedef typename GET_PROP_TYPE(TypeTag, ElementVolumeVariables) ElementVolumeVariables;

    typedef typename GET_PROP_TYPE(TypeTag, GridView) GridView;
    enum { dimWorld = GridView::dimensionworld };
    typedef Dune::FieldVector<Scalar, dimWorld> Vector;

    typedef typename GET_PROP_TYPE(TypeTag, OnePIndices) Indices;
    enum { pressureIdx = Indices::pressureIdx };

public:

    /*!
     * \brief Constructor. Sets the upwind weight.
     */
    OnePLocalResidual()
    {
        // retrieve the upwind weight for the mass conservation equations. Use the value
        // specified via the property system as default, and overwrite
        // it by the run-time parameter from the Dune::ParameterTree
        upwindWeight_ = GET_PARAM(TypeTag, Scalar, UpwindWeight);
    };

    /*!
     * \brief Evaluate the rate of change of all conservation
     *        quantites (e.g. phase mass) within a sub-control
     *        volume of a finite volume element for the OneP
     *        model.
     *
     * This function should not include the source and sink terms.
     *  \param result The phase mass within the sub-control volume
     *  \param scvIdx The SCV (sub-control-volume) index
     *  \param usePrevSol Evaluate function with solution of current or previous time step
     */
    void computeStorage(PrimaryVariables &result, int scvIdx, bool usePrevSol) const
    {
        // if flag usePrevSol is set, the solution from the previous
        // time step is used, otherwise the current solution is
        // used. The secondary variables are used accordingly.  This
        // is required to compute the derivative of the storage term
        // using the implicit euler method.
        const ElementVolumeVariables &elemCtx = usePrevSol ? this->prevVolVars_() : this->curVolVars_();
        const VolumeVariables &volVars = elemCtx[scvIdx];

        // partial time derivative of the wetting phase mass
        result[pressureIdx] =  volVars.density() * volVars.porosity();
    }


    /*!
     * \brief Evaluate the mass flux over a face of a sub-control
     *        volume.
     *
     * \param flux The flux over the SCV (sub-control-volume) face
     * \param faceIdx The index of the SCV face
     */
    void computeFlux(PrimaryVariables &flux, int faceIdx) const
    {
        FluxVariables fluxVars(this->problem_(),
                               this->elem_(),
                               this->fvElemGeom_(),
                               faceIdx,
                               this->curVolVars_());
        Vector tmpVec;
        fluxVars.intrinsicPermeability().mv(fluxVars.potentialGrad(),
                                            tmpVec);

        Scalar normalFlux = -(tmpVec*fluxVars.face().normal);

        const VolumeVariables &up = this->curVolVars_(fluxVars.upstreamIdx(normalFlux));
        const VolumeVariables &dn = this->curVolVars_(fluxVars.downstreamIdx(normalFlux));
        flux[pressureIdx] =
            ((    upwindWeight_)*(up.density()/up.viscosity())
             +
             (1 - upwindWeight_)*(dn.density()/dn.viscosity()))
            *
            normalFlux;
    }

    /*!
     * \brief Calculate the source term of the equation.
     *
     * \param q The source/sink in the SCV
     * \param localVertexIdx The index of the SCV
     *
     */
    void computeSource(PrimaryVariables &q, int localVertexIdx)
    {
        this->problem_().boxSDSource(q,
                                     this->elem_(),
                                     this->fvElemGeom_(),
                                     localVertexIdx,
                                     this->curVolVars_());
    }

    /*!
     * \brief Return the temperature given the solution vector of a
     *        finite volume.
     */
    template <class PrimaryVariables>
    Scalar temperature(const PrimaryVariables &sol)
    { return this->problem_.temperature(); /* constant temperature */ }

private:
    ThisType &asImp_()
    { return *static_cast<ThisType *>(this); }

    const ThisType &asImp_() const
    { return *static_cast<const ThisType *>(this); }

    Scalar upwindWeight_;
};

}

#endif
