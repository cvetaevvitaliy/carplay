
arg_dic	=	{
	'b' : { 'in' : 'gboolean', 'out' : 'gboolean *', 'gtype' : 'G_TYPE_BOOLEAN', 'format' : 'b', 'default' : 'FALSE' },
	'y' : { 'in' : 'guchar', 'out' : 'guchar *', 'gtype' : 'G_TYPE_UCHAR', 'format' : 'y', 'default' : '0' },
	'n' : { 'in' : 'gint16', 'out' : 'gint16 *', 'gtype' : 'G_TYPE_INT', 'format' : 'n', 'default' : '0' },
	'q' : { 'in' : 'guint16', 'out' : 'guint16 *', 'gtype' : 'G_TYPE_UINT', 'format' : 'q', 'default' : '0' },
	'i' : { 'in' : 'gint', 'out' : 'gint *', 'gtype' : 'G_TYPE_INT', 'format' : 'i', 'default' : '0' },
	'u' : { 'in' : 'guint', 'out' : 'guint *', 'gtype' : 'G_TYPE_UINT', 'format' : 'u', 'default' : '0' },
	'x' : { 'in' : 'gint64', 'out' : 'gint64 *', 'gtype' : 'G_TYPE_INT64', 'format' : 'x', 'default' : '0' },
	't' : { 'in' : 'guint64', 'out' : 'guint64 *', 'gtype' : 'G_TYPE_UINT64', 'format' : 't', 'default' : '0' },
	'd' : { 'in' : 'gdouble', 'out' : 'gdouble *', 'gtype' : 'G_TYPE_DOUBLE', 'format' : 'd', 'default' : '0.0' },
	's' : { 'in' : 'const gchar *', 'out' : 'gchar **', 'gtype' : 'G_TYPE_STRING', 'format' : 's', 'default' : '""' },
	'o' : { 'in' : 'const gchar *', 'out' : 'gchar **', 'gtype' : 'G_TYPE_STRING', 'format' : 'o', 'default' : '""' },
	'g' : { 'in' : 'const gchar *', 'out' : 'gchar **', 'gtype' : 'G_TYPE_STRING', 'format' : 'g', 'default' : '""' },
	'ay' : { 'in' : 'const gchar *', 'out' : 'gchar **', 'gtype' : 'G_TYPE_STRING', 'format' : '^ay', 'default' : '""' },
	'as' : { 'in' : 'const gchar * const *', 'out' : 'gchar ***', 'gtype' : 'G_TYPE_STRV', 'format' : '^as', 'default' : '""' },
	'ao' : { 'in' : 'const gchar * const *', 'out' : 'gchar ***', 'gtype' : 'G_TYPE_STRV', 'format' : '^ao', 'default' : '""' },
	'aay' : { 'in' : 'const gchar * const *', 'out' : 'gchar ***', 'gtype' : 'G_TYPE_STRV', 'format' : '^aay', 'default' : '""' },
	'p' : { 'in' : 'gpointer', 'out' : 'gpointer *', 'gtype' : 'G_TYPE_POINTER', 'format' : 'u', 'default' : 'NULL' }
}

class Arg:
	def __init__( self, name, direction, sig ):
		self.name	=	name
		self.sig	=	sig
		self.direction	=	direction

	def post_process( self ):
		pass

class Io:
	def __init__( self, name, override ):
		self.name	=	name
		self.override	=	override

		self.in_args	=	[]
		self.out_args	=	[]

	def post_process( self ):
		for a in self.in_args:
			a.post_process()
		for a in self.out_args:
			a.post_process()

class Event:
	def __init__( self, name ):
		self.name	=	name

		self.in_args	=	[]
		self.out_args	=	[]

	def post_process( self ):
		for a in self.in_args:
			a.post_process()
		for a in self.out_args:
			a.post_process()

class Method:
	def __init__( self, name, override ):
		self.name	=	name
		self.override	=	override

		self.in_args	=	[]
		self.out_args	=	[]

	def post_process( self ):
		for a in self.in_args:
			a.post_process()
		for a in self.out_args:
			a.post_process()

class Signal:
	def __init__( self, name ):
		self.name	=	name
		self.in_args	=	[]

	def post_process( self ):
		for a in self.in_args:
			a.post_process()

class Property:
	def __init__( self, name, scope, ptype ):
		self.name	=	name
		self.scope	=	scope
		self.ptype	=	ptype

	def post_process( self ):
		pass

class Klass:
	def __init__( self, name, parent, parent_header ):
		self.ios		=	[]
		self.events		=	[]
		self.methods	=	[]
		self.signals	=	[]
		self.properties	=	[]

		self.name	=	name
		self.parent	=	parent
		self.parent_header	=	parent_header

	def post_process( self ):
		for m in self.ios:
			m.post_process()

		for s in self.signals:
			s.post_process()

		for p in self.properties:
			p.post_process()
