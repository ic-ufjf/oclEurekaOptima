import bz2
from cStringIO import StringIO
from array import array
import numpy
from scipy import stats
import sys
import warnings
#
#
# Global settings
#
warnings.simplefilter("ignore")
algorithm_names = ( "Best", "TBest", "Rand", "TRand", "SMDE k=2", "SMDE r=0.001", "SMDE w=0", "SMDE w=5", "SMDE RBF" )
folder = "./results/ECT/scripts/results/"
#
# end -- settings
#

#
# Function to read the results
#
def read_results( parameters ):

	file_str = StringIO()
	file_str.write( folder )
	file_str.write( "de_metamodel_apm_" ) #de_metamodel_apm_-61_LLR_10_0-1-2-5_0.9_4_5.out.bz2
	file_str.write( parameters )
	file_str.write( ".out.bz2" )
	bz2_file = bz2.BZ2File( file_str.getvalue(), "r" )

	id_first_readble_line = 1
	text_list = bz2_file.readlines()
	i = len( text_list ) - 1 #i = len( text_list ) - 2 #the last line should not be read
	objective_function_value=0
	constraint_violation_value=1

	j = 0
	while i >= id_first_readble_line and constraint_violation_value > 0:
		#print text_list[ i ]
		#line_list = text_list[ i ].split("\t")
		line_list = text_list[ i ].split(" ")
		j = 0
		while ( j < len( line_list ) and not ( line_list[ j ] == "valid" or line_list[ j ] == "invalid" ) ):
			j+=1

		j+=1
		while ( line_list[ j ] == "" ):
			j += 1
			
		constraint_violation_value = float( line_list[ j ].replace( ",", "." ) )
		#print "constraint_violation_value"
		#print constraint_violation_value
		
		j += 1
		while ( line_list[ j ] == "" ):
			j += 1
		objective_function_value = float( line_list[ j ].replace( ",", "." ) )
		#print "objective_function_value"
		#print objective_function_value
		
		i-=1

	bz2_file.close()
	return array('d', [ objective_function_value, constraint_violation_value ])
#
# end - function
#
#
# Function to create an array with the results of all independent runs
# The resulting array is composed only by feasible solutions
#
def create_array_of_results( problem, metamodel, pm, variants, f ):

	#-60_LLR_10_0-1-2-5_0.9_4_5
	file_str = StringIO()
	file_str.write( str( problem ) )
	file_str.write( "_" )
	#file_str.write( str( flag ) )
	#file_str.write( "_" )
	file_str.write( str( metamodel ) )
	file_str.write( "_" )
	file_str.write( str( pm ) )
	file_str.write( "_" )
	file_str.write( str( variants ) )
	file_str.write( "_" )
	file_str.write( str( f ) )
	file_str.write( "_" )
#	file_str.write( "_" )
#	file_str.write( str( r ) )
#	file_str.write( "_false_" )
#	file_str.write( str( v ) )
	if ( variants == "0-1-2-5" ):
		file_str.write( "4" )
	else:
		file_str.write( "1" )
		
	file_str.write( "_" )
	parameters_str = file_str.getvalue()
	
	i = 1
	results = list()
	while i <= 30:
		solution = read_results( parameters_str + str( i ) )
		if not solution[ 1 ] > 0:
			results.append( solution[ 0 ] )
		i += 1
	
	return array( 'd', results )
#
# end - function
#
# ------------------------------------------------------------------------------
#
# base code
#
# list to storage the results

problems = [ -10, -11, -25, -26, -60, -61, -72, -73, -942, -943 ]

for problem in problems:

	print problem

	results = list()

	# Baseline -- problem, metamodel, pm, variants, f
	# best
#	flag=0
#	k=2
#	c=1
#	metamodel=0
#	r=0
#	v=2
	metamodel = "KNN"
	pm = 2
	variants = 0
	f = 0.7
	results.append( create_array_of_results( problem, metamodel, pm, variants, f ) )
	
	# baseline -- TBest
	metamodel = "KNN"
	pm = 2
	variants = 1
	f = 0.6
	results.append( create_array_of_results( problem, metamodel, pm, variants, f ) )
	
	# baseline -- Rand
	metamodel = "KNN"
	pm = 2
	variants = 2
	f = 0.5
	results.append( create_array_of_results( problem, metamodel, pm, variants, f ) )
	
	# baseline -- TRand
	metamodel = "KNN"
	pm = 2
	variants = 5
	f = 0.7
	results.append( create_array_of_results( problem, metamodel, pm, variants, f ) )
	
	# kNN
#	flag=1
#	k=2
#	c=1
#	metamodel=0
#	r=0
#	v=0
	metamodel = "KNN"
	pm = 2
	variants = "0-1-2-5"
	f = 0.7
	results.append( create_array_of_results( problem, metamodel, pm, variants, f ) )

	# rNN
#	flag=1
#	k=2 
#	c=1 
#	metamodel=1
#	r=0.001
#	v=0
	metamodel = "RNN"
	pm = 0.001
	variants = "0-1-2-5"
	f = 0.7
	results.append( create_array_of_results( problem, metamodel, pm, variants, f ) )

	# LLR
#	flag=1
#	k=2 
#	c=1 
#	metamodel=2
#	r=0
#	v=0
	metamodel = "LLR"
	pm = 0
	variants = "0-1-2-5"
	f = 0.6
	results.append( create_array_of_results( problem, metamodel, pm, variants, f ) )
	
	#LLR-W
	metamodel = "LLR"
	pm = "5"
	variants = "0-1-2-5"
	f = 0.6
	results.append( create_array_of_results( problem, metamodel, pm, variants, f ) )
	
	#RBF
	metamodel = "RBF"
	pm = "1"
	variants = "0-1-2-5"
	f = 0.7
	results.append( create_array_of_results( problem, metamodel, pm, variants, f ) )

	# found best algorithm for each case
	id_min=0
	min_value=sys.float_info.max
	id_median=0
	median_value=sys.float_info.max
	id_average=0
	average_value=sys.float_info.max
	id_max=0
	max_value=sys.float_info.max

	j = 0
	for res in results:
		value = numpy.min( res )
		if value < min_value:
			id_min = j
			min_value = value
		
		value = numpy.median( res )
		if value < median_value or ( value == median_value and numpy.average( res ) < average_value ):
			id_median = j
			median_value = value
		
		value = numpy.average( res )
		if value < average_value:
			id_average = j
			average_value = value
		
		value = numpy.max( res )
		if value < max_value:
			id_max = j
			max_value = value

		j += 1

	j = 0
	for res in results:
		try:
			if j == id_median or ( j == id_average and numpy.median( res )==median_value ):
				line="\\rowcolor[gray]{.8}" + algorithm_names[ j ]
			else:
				line=algorithm_names[ j ]
				if stats.kruskal( results[ id_median ] , res )[ 1 ] > 0.05:
					line += "$^{*}$"
	
			line += "\t&\t$"
			if j == id_min or numpy.min( res )==min_value:
				line += "\mathbf{" + str( "%.2f" % numpy.min( res ) ) + "}"
			else:
				line += str( "%.2f" % numpy.min( res ) )
	
			line += "$\t&\t$" 
			if j == id_median or numpy.median( res )==median_value:
				line += "\mathbf{" + str( "%.2f" % numpy.median( res ) ) + "}"
			else:
				line += str( "%.2f" % numpy.median( res ) )
	
			line += "$\t&\t$" 
			if j == id_average or numpy.average( res )==average_value:
				line += "\mathbf{" + str( "%.2f" % numpy.average( res ) ) + "}"
			else:
				line += str( "%.2f" % numpy.average( res ) )
	
	
			line += "$\t&\t$" + str( "%.2e" % numpy.std( res ) )+ "$\t&\t$"
			if j == id_max or numpy.max( res )==max_value:
				line += "\mathbf{" + str( "%.2f" % numpy.max( res ) ) + "}"
			else:
				line += str( "%.2f" % numpy.max( res ) )
	
			line += "$\t&\t$" + str( len( res ) ) + "$\t\\\\"
			print line
		except BaseException as ex:
			print ex
		j += 1
	print "\\hline \\\\ "

#	print results[ 3 ]


