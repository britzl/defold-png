local M = {}

local client_id = "0166969aba5f28b"

local images = {}

function M.refresh(callback)
	local cache_path = sys.get_save_file(sys.get_config("project.title"), "imgurcache")
	local cache = sys.load(cache_path) or {}

	local headers = {
		Authorization = "Client-ID " .. client_id,
	}
	local url = "https://api.imgur.com/3/gallery/search/viral?q=cat&q_type=png&q_size_px=small"
	http.request(url, "GET", function(self, id, response)
		local imgur_response_data
		if response.status == 304 then
			imgur_response_data = cache[url]
		elseif response.status == 200 then
			imgur_response_data = response.response
			cache[url] = imgur_response_data
			sys.save(cache_path, cache)
		end

		if not imgur_response_data then
			callback(false)
			return
		end

		local decoded_response = json.decode(imgur_response_data)
		for _,image_or_album in pairs(decoded_response.data or {}) do
			if not image_or_album.is_album then
				table.insert(images, image_or_album)
			end
		end
		callback(true)
	end, headers)
end

function M.get_images()
	return images
end

return M
