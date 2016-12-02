#include <networking.h>
#include <errno.h>

void
SavePrivateKey( const string& filename, const PrivateKey& key )
{
	// http://www.cryptopp.com/docs/ref/class_byte_queue.html
	ByteQueue queue;
	key.Save(queue);

	Save(filename, queue);
}

void
SavePublicKey( const string& filename, const PublicKey& key )
{
	// http://www.cryptopp.com/docs/ref/class_byte_queue.html
	ByteQueue queue;
	key.Save(queue);

	Save(filename, queue);
}

void
Save( const string& filename, const BufferedTransformation& bt )
{
	// http://www.cryptopp.com/docs/ref/class_file_sink.html
	FileSink file(filename.c_str());

	bt.CopyTo(file);
	file.MessageEnd();
}

void
LoadPrivateKey( const string& filename, PrivateKey& key )
{
	// http://www.cryptopp.com/docs/ref/class_byte_queue.html
	ByteQueue queue;
	AutoSeededRandomPool rng;

	Load(filename, queue);
	key.Load(queue);
	if ( !key.Validate(rng, 3) )
	{
		throw ( "Failed to load private key" );
	}
}

void
LoadPublicKey( const string& filename, PublicKey& key )
{
	// http://www.cryptopp.com/docs/ref/class_byte_queue.html
	ByteQueue queue;
	AutoSeededRandomPool rng;

	Load(filename, queue);
	key.Load(queue);
	if ( !key.Validate(rng, 3) )
	{
		throw ( "Failed to load public key" );
	}
}

void
Load( const string& filename, BufferedTransformation& bt )
{
	// http://www.cryptopp.com/docs/ref/class_file_source.html
	FileSource file(filename.c_str(), true /*pumpAll*/);

	file.TransferTo(bt);
	bt.MessageEnd();
}
