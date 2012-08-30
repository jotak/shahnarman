cost = 4
texture = "CouncilHall"
ethnicity = ""

function getName()
	if language == "french" then
		return "Salle du Conseil"
	else
		return "Council hall"
	end
end

function getDescription()
	if language == "french" then
		return "+2 influence."
	else
		return "+2 influence."
	end
end

function getMod_radius(val)
	return val + 2
end
