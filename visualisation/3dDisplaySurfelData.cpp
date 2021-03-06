/**
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 **/
/**
 * @file 3dDisplaySurfelData.cpp
 * @author Bertrand Kerautret (\c kerautre@loria.fr )
 * LORIA (CNRS, UMR 7503), University of Nancy, France
 *
 * @date 2014/08/16
 *
 * Display surfel data from SDP file with color attributes given as scalar interpreted as color.
 *
 * This file is part of the DGtal library.
 */

///////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <QGLViewer/qglviewer.h>
#include <stdio.h>

#include "DGtal/base/Common.h"
#include "DGtal/helpers/StdDefs.h"
#include "DGtal/io/viewers/Viewer3D.h"
#include "DGtal/io/DrawWithDisplay3DModifier.h"
#include "DGtal/io/readers/PointListReader.h"
#include "DGtal/io/readers/MeshReader.h"
#include "DGtal/io/colormaps/GradientColorMap.h"

#include "DGtal/topology/helpers/Surfaces.h"
#include "DGtal/topology/SurfelAdjacency.h"
#include "DGtal/topology/CanonicCellEmbedder.h"
#include "DGtal/math/Statistic.h"

#include "DGtal/io/Color.h"
#include "DGtal/io/colormaps/GradientColorMap.h"
#include "DGtal/io/readers/GenericReader.h"

#include "CLI11.hpp"

#include <limits>

using namespace std;
using namespace DGtal;
using namespace Z3i;


/**
 @page Doc3dDisplaySurfelData 3dDisplaySurfelData
 
 @brief  Displays surfel data from SDP file with color attributes given as scalar interpreted as color. 

 @b Usage:   3dDisplaySurfelData [OPTIONS] 1 [fixMaxColorValue] [fixMinColorValue]

 

 @b Allowed @b options @b are :
 
 @code

 Positionals:
   1 TEXT:FILE REQUIRED                  input file: sdp (sequence of discrete points with attribute)
  
 Options:
   -h,--help                             Print this help message and exit
   -i,--input TEXT:FILE REQUIRED         input file: sdp (sequence of discrete points with attribute)
   -n,--noWindows                        Don't display Viewer windows.
   -d,--doSnapShotAndExit TEXT           save display snapshot into file.
   --labelIndex UINT                     set the index of the label (by default set to 3)
   --SDPindex UINT ...                   specify the sdp index (by default 0,1,2).
   --fixMaxColorValue FLOAT              fix manually the maximal color value for the scale error display (else the scale is set from the maximal value)
  --fixMinColorValue FLOAT               fix manually the maximal color value for the scale error display (else the scale is set from the minimal value)

 @endcode


 @b Example: 
 
 To test this program we have first to generate a file containing a set of surfels with, for instance, their associated curvature values:
 @code
$ 3dCurvatureViewer -i $DGtal/examples/samples/cat10.vol -r 3 --exportOnly -d curvatureCat10R3.dat
 @endcode

 Then, we can use this tool to display the set of surfel with their associated values:
 @code
$ 3dDisplaySurfelData -i curvatureCat10R3.dat 
 @endcode 
 You should obtain such a result:

 @image html res3dDisplaySurfelData.png "resulting visualisation of surfel with their associated values."
 

 @see
 @ref 3dDisplaySurfelData.cpp, @ref CompSurfelData

 */



template < typename Space = DGtal::Z3i::Space, typename KSpace = DGtal::Z3i::KSpace>
struct ViewerSnap: DGtal::Viewer3D <Space, KSpace>
{

  ViewerSnap(const KSpace &KSEmb, bool saveSnap): Viewer3D<Space, KSpace>(KSEmb), mySaveSnap(saveSnap){
  };

  virtual  void
  init(){
    DGtal::Viewer3D<>::init();
    if(mySaveSnap)
    {
      QObject::connect(this, SIGNAL(drawFinished(bool)), this, SLOT(saveSnapshot(bool)));
    }
  };
  bool mySaveSnap;
};


template < typename Point>
void
getBoundingUpperAndLowerPoint(const std::vector<Point> &vectorPt, Point &ptLower, Point &ptUpper){
  for(unsigned int i =1; i<vectorPt.size(); i++)
  {
    if(vectorPt.at(i)[0] < ptLower[0])
    {
      ptLower[0] = vectorPt.at(i)[0];
    }
    if(vectorPt.at(i)[1] < ptLower[1])
    {
      ptLower[1] = vectorPt.at(i)[1];
    }
   if(vectorPt.at(i)[2] < ptLower[2])
   {
      ptLower[2] =vectorPt.at(i)[2];
    }
   if(vectorPt.at(i)[0] < ptLower[0])
   {
      ptLower[0] = vectorPt.at(i)[0];
   }
   if(vectorPt.at(i)[1] < ptLower[1])
   {
     ptLower[1] = vectorPt.at(i)[1];
    }
   if(vectorPt.at(i)[2] < ptLower[2])
   {
      ptLower[2] =vectorPt.at(i)[2];
    }
  }
}


int main( int argc, char** argv )
{
  typedef PointVector<4, double> Point4D;
  typedef PointVector<1, int> Point1D;
  
  // parse command line using CLI ----------------------------------------------
  CLI::App app;
  std::string inputFileName;
  std::string doSnapShotName;
  bool noWindows {false};
  double minScalarVal;
  double maxScalarVal;
  unsigned int labelIndex;
  std::vector<unsigned int> vectSDPindex {0, 1, 2};
  
  app.description("Display surfel data from SDP file with color attributes given as scalar interpreted as color. \n Example of use: \n First you have to generate a file containing a set of surfels with, for instance, their associated curvature values: \n 3dCurvatureViewer -i $DGtal/examples/samples/cat10.vol -r 3 --exportOnly -d curvatureCat10R3.dat \n Then, we can use this tool to display the set of surfel with their associated values: 3dDisplaySurfelData -i curvatureCat10R3.dat");
  app.add_option("-i,--input,1", inputFileName, "input file: sdp (sequence of discrete points with attribute)" )
  ->required()
  ->check(CLI::ExistingFile);
  app.add_flag("--noWindows,-n",noWindows, "Don't display Viewer windows.");
  app.add_option("--doSnapShotAndExit,-d", doSnapShotName, "save display snapshot into file.");
  auto fixMaxOption =  app.add_option("--fixMaxColorValue,-M", maxScalarVal, "fix manually the maximal color value for the scale error display (else the scale is set from the maximal value)");
  auto fixMinOption =  app.add_option("--fixMinColorValue,-m", minScalarVal, "fix manually the maximal color value for the scale error display (else the scale is set from the minimal value)");
  app.add_option("--labelIndex", labelIndex , "set the index of the label.", true);
  app.add_option("--SDPindex", vectSDPindex, "specify the sdp index.", true);
  
  app.get_formatter()->column_width(40);
  CLI11_PARSE(app, argc, argv);
  // END parse command line using CLI ----------------------------------------------

  std::vector<Point4D> surfelAndScalarInput;
  Z3i::KSpace K;

  surfelAndScalarInput = PointListReader<Point4D>::getPointsFromFile(inputFileName, vectSDPindex);

  Point4D ptLower = surfelAndScalarInput.at(0);
  Point4D ptUpper = surfelAndScalarInput.at(0);
  getBoundingUpperAndLowerPoint(surfelAndScalarInput,  ptLower, ptUpper);
  
  K.init(Z3i::Point(2*ptLower[0]+1, 2*ptLower[1]+1, 2*ptLower[2]+1),
         Z3i::Point(2*ptUpper[0]+1, 2*ptUpper[1]+1, 2*ptUpper[2]+1), true);


  std::vector<Cell> vectSurfelsInput;

  // Construction of the set of surfels
  for(unsigned int i =0; i<surfelAndScalarInput.size(); i++){
    Point4D pt4d = surfelAndScalarInput.at(i);
    Cell c = K.uCell(Z3i::Point(pt4d[0], pt4d[1], pt4d[2]));
    vectSurfelsInput.push_back(c);
  }

  CanonicCellEmbedder<KSpace> embeder(K);
  std::vector<unsigned int> vectIndexMinToReference;

  //-------------------------
  // Displaying input with color given from scalar values

  QApplication application(argc,argv);
  typedef ViewerSnap<> Viewer;

  Viewer viewer(K, doSnapShotName != "");
  if(doSnapShotName != "")
  {
    viewer.setSnapshotFileName(QString(doSnapShotName.c_str()));
  }
  viewer.setWindowTitle("3dCompSurfel Viewer");
  viewer.show();
  viewer.restoreStateFromFile();
  
  if( fixMinOption->count() == 0 )
  {
    minScalarVal=surfelAndScalarInput.at(0)[3];
  }
  if( fixMaxOption->count() == 0 )
  {
    maxScalarVal=surfelAndScalarInput.at(0)[3];
  }
  
  if( fixMinOption->count() == 0 || fixMaxOption->count() == 0)
  {
    for(unsigned int i=1; i <surfelAndScalarInput.size(); i++)
    {
      double scalVal = surfelAndScalarInput.at(i)[3];
      if(scalVal < minScalarVal  && fixMinOption->count() == 0)
      {
        minScalarVal = scalVal;
      }
      if(scalVal > maxScalarVal && fixMaxOption->count() == 0)
      {
        maxScalarVal = scalVal;
      }
    }
  }
  
  GradientColorMap<double> gradientColorMap( minScalarVal, maxScalarVal );
  gradientColorMap.addColor( Color(255,0,0,100 ) );
  gradientColorMap.addColor( Color(0,255,0,100 ) );
  gradientColorMap.addColor( Color(0,0,255,100 ) );

  bool useGrad = minScalarVal!=maxScalarVal;

  viewer << SetMode3D(vectSurfelsInput.at(0).className(), "Basic");
  for(unsigned int i=0; i <surfelAndScalarInput.size(); i++)
  {
    double valInput = surfelAndScalarInput.at(i)[3];
    if(useGrad)
    {
      viewer.setFillColor(gradientColorMap(valInput));
    }else
    {
      viewer.setFillColor(Color::White);
    }
    viewer << vectSurfelsInput.at(i);
  }



  viewer << Viewer::updateDisplay;
  if(doSnapShotName != "")
  {
    // Appy cleaning just save the last snap
    std::string name = doSnapShotName;
    std::string extension = name.substr(name.find_last_of(".") + 1);
    std::string basename = name.substr(0, name.find_last_of("."));
    for(int i=0; i< viewer.snapshotCounter()-1; i++){
      std::stringstream s;
      s << basename << "-"<< setfill('0') << setw(4)<<  i << "." << extension;
      trace.info() << "erase temp file: " << s.str() << std::endl;
      remove(s.str().c_str());
    }
    std::stringstream s;
    s << basename << "-"<< setfill('0') << setw(4)<<  viewer.snapshotCounter()-1 << "." << extension;
    rename(s.str().c_str(), name.c_str());
    return 0;
  }

  if(noWindows)
  {
    return 0;
  }
  else
  {
    return application.exec();
  }
}
