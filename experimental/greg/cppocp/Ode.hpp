// Ode.hpp
// Greg Horn
// Casadi 2011

#pragma once

#include <string>
#include <map>

#include <core/sx/sx_tools.hpp>
#include <core/function/sx_function.hpp>

#define IDX_ODE_STEP_STATE  0
#define IDX_ODE_STEP_ACTION 1
#define IDX_ODE_STEP_T0     2
#define IDX_ODE_STEP_DT     3
#define NUM_ODE_STEP_INPUTS 4

class Ode
{

public:
     ~Ode(void);
     Ode(std::string _name);

     std::string name;

     // states/actions/outputs
     std::map<std::string,int> states;
     std::map<std::string,int> actions;
     std::map<std::string,int> outputs;
	
     void addState(std::string _newState);
     void addAction(std::string _newAction);
     void addOutput(std::string _newOutput);

     void (*dxdt)(std::map<std::string,casadi::SX> &xDot,
		  std::map<std::string,casadi::SX> &outputs,
		  std::map<std::string,casadi::SX> state,
		  std::map<std::string,casadi::SX> action,
		  std::map<std::string,casadi::SX> param,
		  casadi::SX t);

     void assertUnlocked(void);
     void init(void);
  void setupIntegrators(void);
  void setupIntegrators(std::map<std::string,double> & params);

     std::map<std::string,casadi::SX> getOutputFromDxdt( casadi::SX x,
							 casadi::SX u,
							 std::map<std::string,casadi::SX> & p,
							 casadi::SX t );

     casadi::SX dxVectorDt( casadi::SX x,
				  casadi::SX u,
				  std::map<std::string,casadi::SX> & p,
				  casadi::SX t );

     casadi::DMatrix rk4Step( casadi::DMatrix & xk,
			      casadi::DMatrix & uk,
			      double t0,
			      double dt);

     casadi::DMatrix eulerStep( casadi::DMatrix & xk,
				casadi::DMatrix & uk,
				double t0,
				double dt);

     casadi::SX rk4Step( casadi::SX x0Vec,
			       casadi::SX u0Vec,
			       casadi::SX u1Vec,
			       std::map<std::string,casadi::SX> & p,
			       casadi::SX t0,
			       casadi::SX dt);

     casadi::SX eulerStep( casadi::SX x0Vec,
				 casadi::SX u0Vec,
				 std::map<std::string,casadi::SX> & p,
				 casadi::SX t0,
				 casadi::SX dt);
	
     casadi::SX simpsonsRuleError( casadi::SX x0Vec,
					 casadi::SX x1Vec,
					 casadi::SX u0Vec,
					 casadi::SX u1Vec,
					 std::map<std::string,casadi::SX> & p,
					 casadi::SX t0,
					 casadi::SX dt);

     int nx(void);
     int nu(void);
     int nxu(void);
     int no(void);

     void assertUniqueName(std::string newName);

     std::map<std::string,casadi::SX> getStateMap( casadi::SX & x);
     std::map<std::string,casadi::SX> getActionMap( casadi::SX & u);

private:
     int isState(std::string stateName);
     int isAction(std::string actionName);
     int isOutput(std::string outputName);
     int locked;

     casadi::SXFunction rk4StepFcn;
     casadi::SXFunction eulerStepFcn;

protected:
};
