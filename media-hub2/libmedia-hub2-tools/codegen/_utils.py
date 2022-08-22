def klass_type( name ):
	_split	=	name.split( '_' )
	_split.insert( 1, 'type' )

	return '_'.join( _split ).upper()

def obj_cast( name ):
	return name.upper()

def obj_check( name ):
	_split	=	name.split( '_' )
	_split.insert( 1, 'is' )

	return '_'.join( _split ).upper()

def obj_name( name ):
	_split	=	name.split( '_' )
	_tmp	=	_split[0].upper()

	for s in _split[1:]:
		_tmp	+=	s.title()

	return _tmp

def klass_cast( name ):
	_split	=	name.split( '_' )
	_split.append( 'class' )

	return '_'.join( _split ).upper()

def klass_check( name ):
	_split	=	name.split( '_' )
	_split.insert( 1, 'is' )
	_split.append( 'class' )

	return '_'.join( _split ).upper()

def klass_get( name ):
	return name.upper() + '_GET_CLASS'

def klass_name( name ):
	return obj_name( name ) + 'Class'
