import sys
import xml.parsers.expat

import _types

class PluginXMLParser:
	STATE_TOP		=	'top'
	STATE_NODE		=	'node'
	STATE_KLASS		=	'klass'
	STATE_PARENT	=	'parent'
	STATE_IO	=	'io'
	STATE_EVENT		=	'event'
	STATE_METHOD	=	'method'
	STATE_PROPERTY	=	'property'
	STATE_SIGNAL	=	'signal'
	STATE_ARG		=	'arg'
	STATE_IGNORED	=	'ignored'

	def __init__( self, xml_data ):
		self._parser	=	xml.parsers.expat.ParserCreate()

		self._parser.CommentHandler			=	self.handle_comment
		self._parser.CharacterDataHandler	=	self.handle_char_data
		self._parser.StartElementHandler	=	self.handle_start_element
		self._parser.EndElementHandler		=	self.handle_end_element

		self.klass	=	[]
		self.cur_obj	=	None

		self.state	=	PluginXMLParser.STATE_TOP
		self.state_stack	=	[]
		self.cur_obj_stack		=	[]

		self._parser.Parse( xml_data )

	def handle_comment( self, data ):
		pass

	def handle_char_data( self, data ):
		pass

	def handle_start_element( self, name, attrs ):
		old_state	=	self.state
		old_cur		=	self.cur_obj

		if self.state == PluginXMLParser.STATE_TOP:
			if name == PluginXMLParser.STATE_NODE:
				self.state	=	PluginXMLParser.STATE_NODE

			else:
				print( 'error occured %s'%self.state )
				self.state	=	PluginXMLParser.STATE_IGNORED

		elif self.state == PluginXMLParser.STATE_NODE:
			if name == PluginXMLParser.STATE_KLASS:
				self.state		=	PluginXMLParser.STATE_KLASS
				_klass			=	_types.Klass( attrs[ 'name' ], attrs[ 'parent' ],
										attrs[ 'parent_header'] )
				self.klass.append( _klass );
				self.cur_obj	=	_klass;

			else:
				print( 'error occured %s'%self.state )
				self.state	=	PluginXMLParser.STATE_IGNORED

		elif self.state == PluginXMLParser.STATE_KLASS:
			if name == PluginXMLParser.STATE_IO:
				self.state		=	PluginXMLParser.STATE_IO
				_io			=	_types.Io( attrs[ 'name' ], attrs.get( 'override' ) )
				self.cur_obj.ios.append( _io )
				self.cur_obj	=	_io

			elif name == PluginXMLParser.STATE_EVENT:
				self.state		=	PluginXMLParser.STATE_EVENT
				_event			=	_types.Event( attrs[ 'name' ] )
				self.cur_obj.events.append( _event )
				self.cur_obj	=	_event

			elif name == PluginXMLParser.STATE_METHOD:
				self.state		=	PluginXMLParser.STATE_METHOD
				_method			=	_types.Method( attrs[ 'name' ], attrs.get( 'override' )  )
				self.cur_obj.methods.append( _method )
				self.cur_obj	=	_method

			elif name == PluginXMLParser.STATE_PROPERTY:
				self.state	=	PluginXMLParser.STATE_PROPERTY
				_property	=	_types.Property( attrs[ 'name' ], attrs[ 'scope' ], attrs[ 'type' ] )
				self.cur_obj.properties.append( _property )
				self.cur_obj	=	_property

			elif name == PluginXMLParser.STATE_SIGNAL:
				self.state	=	PluginXMLParser.STATE_SIGNAL
				_signal		=	_types.Signal( attrs[ 'name' ] )
				self.cur_obj.signals.append( _signal )
				self.cur_obj	=	_signal

			else:
				print( 'error occured %s %s'%( self.state, name ))
				self.state	=	PluginXMLParser.STATE_IGNORED

		elif self.state == PluginXMLParser.STATE_IO:
			if name == PluginXMLParser.STATE_ARG:
				self.state	=	PluginXMLParser.STATE_ARG
				_arg	=	_types.Arg( attrs[ 'name' ], attrs[ 'direction' ], attrs[ 'type' ] )

				if attrs[ 'direction' ] == 'in':
					self.cur_obj.in_args.append( _arg )
				else:
					self.cur_obj.out_args.append( _arg )

				self.cur_obj	=	_arg

			else:
				print( 'error occured %s'%self.state )

		elif self.state == PluginXMLParser.STATE_EVENT:
			if name == PluginXMLParser.STATE_ARG:
				self.state	=	PluginXMLParser.STATE_ARG
				_arg	=	_types.Arg( attrs[ 'name' ], attrs[ 'direction' ], attrs[ 'type' ] )

				if attrs[ 'direction' ] == 'in':
					self.cur_obj.in_args.append( _arg )
				else:
					self.cur_obj.out_args.append( _arg )

				self.cur_obj	=	_arg

			else:
				print( 'error occured %s'%self.state )

		elif self.state == PluginXMLParser.STATE_METHOD:
			if name == PluginXMLParser.STATE_ARG:
				self.state	=	PluginXMLParser.STATE_ARG
				_arg	=	_types.Arg( attrs[ 'name' ], attrs[ 'direction' ], attrs[ 'type' ] )

				if attrs[ 'direction' ] == 'in':
					self.cur_obj.in_args.append( _arg )
				else:
					self.cur_obj.out_args.append( _arg )

				self.cur_obj	=	_arg

			else:
				print( 'error occured %s'%self.state )
		elif self.state == PluginXMLParser.STATE_SIGNAL:
			if name == PluginXMLParser.STATE_ARG:
				self.state	=	PluginXMLParser.STATE_ARG
				_arg	=	_types.Arg( attrs[ 'name' ], 'in', attrs[ 'type' ] )
				self.cur_obj.in_args.append( _arg )
				self.cur_obj	=	_arg

			else:
				print( 'error occured %s'%self.state )

		self.state_stack.append( old_state )
		self.cur_obj_stack.append( old_cur )

	def handle_end_element(self, name):
		self.state		=	self.state_stack.pop()
		self.cur_obj	=	self.cur_obj_stack.pop()

def parse_plugin_xml( xml_data ):
	parser	=	PluginXMLParser( xml_data )
	return parser.klass
