cost = 4
texture = "Shipyard"
ethnicity = ""

function getName()
	if language == "french" then
		return "Chantier naval"
	else
		return "Shipyard"
	end
end

function getDescription()
	if language == "french" then
		return "(TODO) permet de produire des embarcations."
	else
		return "(TODO) allows to produce ships."
	end
end

