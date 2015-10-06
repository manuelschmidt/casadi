/*
 *    This file is part of CasADi.
 *
 *    CasADi -- A symbolic framework for dynamic optimization.
 *    Copyright (C) 2010-2014 Joel Andersson, Joris Gillis, Moritz Diehl,
 *                            K.U. Leuven. All rights reserved.
 *    Copyright (C) 2011-2014 Greg Horn
 *
 *    CasADi is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 3 of the License, or (at your option) any later version.
 *
 *    CasADi is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with CasADi; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#include "implicit_to_nlp.hpp"

#include "casadi/core/function/mx_function.hpp"

using namespace std;
namespace casadi {

  extern "C"
  int CASADI_IMPLICITFUNCTION_NLP_EXPORT
  casadi_register_implicitfunction_nlp(ImplicitFunctionInternal::Plugin* plugin) {
    plugin->creator = QpToImplicit::creator;
    plugin->name = "nlp";
    plugin->doc = QpToImplicit::meta_doc.c_str();
    plugin->version = 23;
    plugin->adaptorHasPlugin = NlpSolver::hasPlugin;
    return 0;
  }

  extern "C"
  void CASADI_IMPLICITFUNCTION_NLP_EXPORT casadi_load_implicitfunction_nlp() {
    ImplicitFunctionInternal::registerPlugin(casadi_register_implicitfunction_nlp);
  }

  QpToImplicit::QpToImplicit(const std::string& name, const Function& f)
    : ImplicitFunctionInternal(name, f) {

    Adaptor<QpToImplicit, NlpSolverInternal>::addOptions();
  }

  QpToImplicit::~QpToImplicit() {
  }

  void QpToImplicit::solveNonLinear() {

    // Equality nonlinear constraints
    solver_.input(NLP_SOLVER_LBG).set(0.);
    solver_.input(NLP_SOLVER_UBG).set(0.);

    // Simple constraints
    vector<double>& lbx = solver_.input(NLP_SOLVER_LBX).data();
    vector<double>& ubx = solver_.input(NLP_SOLVER_UBX).data();
    for (int k=0; k<u_c_.size(); ++k) {
      lbx[k] = u_c_[k] <= 0 ? -std::numeric_limits<double>::infinity() : 0;
      ubx[k] = u_c_[k] >= 0 ?  std::numeric_limits<double>::infinity() : 0;
    }

    // Pass initial guess
    solver_.input(NLP_SOLVER_X0).set(output(iout_));

    // Add auxiliary inputs
    vector<double>::iterator nlp_p = solver_.input(NLP_SOLVER_P).begin();
    for (int i=0; i<nIn(); ++i) {
      if (i!=iin_) {
        std::copy(input(i).begin(), input(i).end(), nlp_p);
        nlp_p += input(i).nnz();
      }
    }

    // Solve the NLP
    solver_.evaluate();
    stats_["solver_stats"] = solver_.getStats();

    // Get the implicit variable
    output(iout_).set(solver_.output(NLP_SOLVER_X));

    // Evaluate auxilary outputs, if necessary
    if (nOut()>0) {
      f_.setInput(output(iout_), iin_);
      for (int i=0; i<nIn(); ++i)
        if (i!=iin_) f_.setInput(input(i), i);
      f_.evaluate();
      for (int i=0; i<nOut(); ++i) {
        if (i!=iout_) f_.getOutput(output(i), i);
      }
    }
  }

  void QpToImplicit::init() {
    // Call the base class initializer
    ImplicitFunctionInternal::init();

    // Free variable in the NLP
    MX u = MX::sym("u", input(iin_).sparsity());

    // So that we can pass it on to createParent
    std::vector<MX> inputs;
    for (int i=0; i<nIn(); ++i) {
      if (i!=iin_) {
        stringstream ss;
        ss << "p" << i;
        inputs.push_back(MX::sym(ss.str(), input(i).sparsity()));
      }
    }
    MX p = veccat(inputs);

    // Dummy NLP objective
    MX nlp_f = 0;

    // NLP constraints
    std::vector< MX > args_call(nIn());
    args_call[iin_] = u;
    for (int i=0, i2=0; i<nIn(); ++i)
      if (i!=iin_) args_call[i] = inputs[i2++];
    MX nlp_g = f_(args_call).at(iout_);

    // We're going to use two-argument objective and constraints to allow the use of parameters
    MXFunction nlp("nlp", nlpIn("x", u, "p", p), nlpOut("f", nlp_f, "g", nlp_g));

    Dict options;
    if (hasSetOption(optionsname())) options = getOption(optionsname());
    // Create an NlpSolver instance
    solver_ = NlpSolver("nlpsolver", getOption(solvername()), nlp, options);
  }

} // namespace casadi
