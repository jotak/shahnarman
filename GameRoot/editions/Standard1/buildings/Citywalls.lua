cost = 4
texture = "Citywalls"
ethnicity = ""

function getName()
	if language == "french" then
		return "Remparts"
	else
		return "City walls"
	end
end

function getDescription()
	if language == "french" then
		return "(TODO) augmente la défense dans la ville."
	else
		return "(TODO) increase defence in town."
	end
end

