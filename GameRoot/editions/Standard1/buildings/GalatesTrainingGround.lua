cost = 15
texture = "a"
ethnicity = ""

function getName()
	if language == "french" then
		return "Terrain d'entra�nement"
	else
		return "Training ground"
	end
end

function getDescription()
	if language == "french" then
		return "..."
	else
		return "..."
	end
end
