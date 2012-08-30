cost = 4
texture = "Shrine"
ethnicity = ""

function getName()
	if language == "french" then
		return "Autel"
	else
		return "Shrine"
	end
end

function getDescription()
	if language == "french" then
		return "+2 heureux."
	else
		return "+2 happy."
	end
end

function getMod_happy(val)
	return val + 2
end
