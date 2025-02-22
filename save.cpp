if (getHeaders().find("Content-Length") == getHeaders().end())
{
	this->statusCode.code = 411;
	std::cout << "-- Content-Length header missing 411 --" << std::endl;
	return false;
}
else
{
	setContentLength(atoi(getHeader("Content-Length").c_str()));
}
if (getHeaders().find("Transfer-Encoding") != getHeaders().end())
{
	setTransferEncodingExist(true);
}
if (getHeaders().find("Content-Type") == getHeaders().end())
{
	this->statusCode.code = 400;
	std::cout << "-- Content-Type header missing 400 --" << std::endl;
	return false;
}
else
{
	if (getHeader("Content-Type").find("boundary=") != std::string::npos)
	{
		size_t boundary_pos = getHeader("Content-Type").find("boundary=");
		boundary = getHeader("Content-Type").substr(boundary_pos + 9);
		setBoundary(boundary);
		std::cout << "boundary:* " << getBoundary() << std::endl;
		setContent_type("multipart/form-data");
	}
}

						// if (!getHeader("Content-Length").empty())
						// {
						// 	setContentLength(atoi(getHeader("Content-Length").c_str()));
						// 	std::cout << "---Content-Length: " << getContentLength() << std::endl;
						// }
						// if (!getHeader("Transfer-Encoding").empty())
						// {
						// 	setTransferEncodingExist(true);
						// }
						// if (!getHeader("Content-Type").empty())
						// {
						// 	setContent_type(getHeader("Content-Type"));
						// 	std::cout << "----Content-Type: " << getContent_type() << std::endl;
						// }
						// if (getHeader("Content-Type").find("boundary=") != std::string::npos)
						// {
						// 	size_t boundary_pos = getHeader("Content-Type").find("boundary=");
						// 	boundary = getHeader("Content-Type").substr(boundary_pos + 9);
						// 	setBoundary(boundary);
						// 	std::cout << "----boundary:* " << getBoundary() << std::endl;
						// 	setContent_type("multipart/form-data");
						// 	std::cout << "----Content-Type: " << getContent_type() << std::endl;
		
						// }