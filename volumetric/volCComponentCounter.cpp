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
 * @file volCComponentCounter.cpp
 * @ingroup tools
 * @author David Coeurjolly (\c david.coeurjolly@liris.cnrs.fr)
 * LIRIS (CNRS, UMR 5205), University de Lyon, France.
 *
 * @date 2011/05/04
 *
 * A simple program to count the number of connected components in a 3D image..
 *
 * This file is part of the DGtal library.
 */

///////////////////////////////////////////////////////////////////////////////
#include <iostream>

#include "DGtal/base/Common.h"
#include "DGtal/io/readers/VolReader.h"
#include "DGtal/io/Color.h"
#include "DGtal/images/ImageSelector.h"
#include "DGtal/helpers/StdDefs.h"

#include <boost/pending/disjoint_sets.hpp>

#include "CLI11.hpp"

using namespace std;
using namespace DGtal;
using namespace Z3i;


/**
 @page volCComponentCounter volCComponentCounter
 
 @brief Counts the number of connected component (same values) in a  volume (Vol) file image.

 @b Usage:   ./volumetric/volCComponentCounter [OPTIONS] 1


 @b Allowed @b options @b are : 
 @code

 Positionals:
     1 TEXT:FILE REQUIRED                  volume file (.vol).

   Options:
     -h,--help                             Print this help message and exit
     -i,--input TEXT:FILE REQUIRED         volume file (.vol).
     -c,--connectivity UINT:{6,18,26}=6    object connectivity (6,18,26) (default: 6 ).

 @endcode

 @b Example: 

 @code
 $ volCComponentCounter $DGtal/examples/samples/Al.100.vol
 @endcode


 You should obtain:
@verbatim
New Block [Initial disjoint sets construction]
EndBlock [Initial disjoint sets construction] (515.602 ms)
New Block [Merging neighboring sets]
EndBlock [Merging neighboring sets] (1672.21 ms)
Number of disjoint 6-components = 2
@endverbatim

 
 @see
 @ref volCComponentCounter.cpp

 */


template <typename Rank, typename Parent, typename Image>
void CCCounter(Rank& r, Parent& p, const Image& elements, const unsigned int connectivity)
{
  
  boost::disjoint_sets<Rank,Parent> dsets(r, p);
  trace.beginBlock("Initial disjoint sets construction");
  for(typename Image::Domain::ConstIterator e = elements.domain().begin();
      e != elements.domain().end(); ++e)
    dsets.make_set(*e);
  trace.endBlock();

  trace.beginBlock("Merging neighboring sets");
  typename Image::Point decx(1,0,0);
  typename Image::Point decy(0,1,0);
  typename Image::Point decz(0,0,1);
  
  //Merging process
  for ( typename Image::Domain::ConstIterator e = elements.domain().begin();
        e != elements.domain().end(); ++e )
  {
    if ( elements.domain().isInside( *e + decx ) &&
         ( elements( *e ) == elements( *e + decx ) ) )
      dsets.union_set( *e, *e + decx );

    if ( elements.domain().isInside( *e + decy ) &&
         ( elements( *e ) == elements( *e + decy ) ) )
      dsets.union_set( *e, *e + decy );

    if ( elements.domain().isInside( *e + decz ) &&
         ( elements( *e ) == elements( *e + decz ) ) )
      dsets.union_set( *e, *e + decz );

    if ( connectivity > 6 )
    {
      if ( elements.domain().isInside( *e + decx + decy ) &&
           ( elements( *e ) == elements( *e + decx + decy ) ) )
        dsets.union_set( *e, *e + decx + decy );

      if ( elements.domain().isInside( *e + decx + decz ) &&
           ( elements( *e ) == elements( *e + decx + decz ) ) )
        dsets.union_set( *e, *e + decx + decz );

      if ( elements.domain().isInside( *e + decy + decz ) &&
           ( elements( *e ) == elements( *e + decy + decz ) ) )
        dsets.union_set( *e, *e + decy + decz );

      if ( connectivity == 26 )
        if ( elements.domain().isInside( *e + decy + decz + decx ) &&
             ( elements( *e ) == elements( *e + decy + decz + decx ) ) )
          dsets.union_set( *e, *e + decy + decz + decx );
    }
    }
  trace.endBlock();
  std::cout << "Number of disjoint "<<connectivity<<"-components = "
            <<dsets.count_sets(elements.domain().begin(),
                               elements.domain().end())
            << std::endl;
}


int main( int argc, char** argv )
{
    
  // parse command line using CLI ----------------------------------------------
  CLI::App app;
  std::string inputFileName;
  unsigned int connectivity {6};
  
  app.description("Count the number of connected component (same values) in a volume (Vol) file image\n\n Example : \n \t volCComponentCounter $DGtal/examples/samples/Al.100.vol\n");
  app.add_option("-i,--input,1", inputFileName, "volume file (.vol)." )
  ->required()
  ->check(CLI::ExistingFile);
  app.add_option("--connectivity,-c",connectivity,"object connectivity (6,18,26) (default: 6 ).", true)
   -> check(CLI::IsMember({6, 18, 26}));
 
  app.get_formatter()->column_width(40);
  CLI11_PARSE(app, argc, argv);
  // END parse command line using CLI ----------------------------------------------


  typedef ImageSelector<Domain, unsigned char>::Type Image;
  Image image = VolReader<Image>::importVol( inputFileName );

  trace.info() << "Image loaded: "<<image<< std::endl;

  typedef std::map<Point,std::size_t> rank_t; // => order on Element
  typedef std::map<Point,Point> parent_t;
  rank_t rank_map;
  parent_t parent_map;
 
  boost::associative_property_map<rank_t>   rank_pmap(rank_map);
  boost::associative_property_map<parent_t> parent_pmap(parent_map);
 
  CCCounter(rank_pmap, parent_pmap, image, connectivity);
 
  return 0;
}
