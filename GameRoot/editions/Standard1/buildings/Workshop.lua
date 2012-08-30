cost = 4
texture = "Workshop"
ethnicity = ""

function getName()
	if language == "french" then
		return "Atelier"
	else
		return "Workshop"
	end
end

function getDescription()
	if language == "french" then
		return "+10% de production."
	else
		return "+10% production."
	end
end

function getMod_prod(val)
	return val * 1.1
end
