#!/usr/bin/perl

use FindBin '$RealBin';

use lib "$RealBin/tools/perl";

use Compile::Driver::Configuration;
use Compile::Driver::Jobs;
use Compile::Driver::Module;
use Compile::Driver::Options;

use warnings;
use strict;

my @args = Compile::Driver::Options::set_options( @ARGV );

my $configuration = Compile::Driver::Configuration::->new( Compile::Driver::Options::specs );

my @jobs;
my @tasks;


sub build
{
	my ( $module ) = @_;
	
	my $target = $module->target;
	
	my $product = $module->product_type;
	
	my $is_lib = $module->is_static_lib;
	my $is_exe = $module->is_executable;
	
	my $type = $is_lib ? "AR"
	         : $is_exe ? "LINK"
	         :           "";
	
	return if !$type;
	
	my @sources = $module->sources;
	
	my $name = $module->name;
	
	my @objs;
	
	foreach my $path ( @sources )
	{
		my $obj = Compile::Driver::Jobs::obj_pathname( $target, $name, $path );
		
		push @objs, $obj;
		
		my $compile =
		{
			TYPE => "CC",
			FROM => $module,
			PATH => $path,
			DEST => $obj,
		};
		
		push @jobs, $compile;
	}
	
	my $dest = $is_lib ? Compile::Driver::Jobs::lib_pathname( $target, $name )
	                   : Compile::Driver::Jobs::bin_pathname( $target, $name, $module->program_name );
	
	my $link =
	{
		TYPE => $type,
		FROM => $module,
		OBJS => \@objs,
		DEST => $dest,
	};
	
	$link->{PREQ} = $module->recursive_prerequisites  if $is_exe;
	
	push @jobs, $link;
}

foreach my $name ( @args )
{
	my $module = $configuration->get_module( $name, "[mandatory]" );
}

my $desc = { NAME => "[program args]", DATA => { use => [ @args ] } };

my $module = Compile::Driver::Module::->new( $configuration, $desc );

my @prereqs = @{ $module->recursive_prerequisites };

pop @prereqs;

build( $configuration->get_module( $_ ) )  foreach @prereqs;

foreach my $job ( @jobs )
{
	Compile::Driver::Jobs::do_job( $job );
}

